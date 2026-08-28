#!/usr/bin/env python3
"""Convert the flat IEEE merged-case JSON format to this project's SCUC input format."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Convert a merged IEEE SCUC case to UnitCommitment JSON.")
    parser.add_argument("--input", type=Path, required=True, help="Source merged-case JSON file")
    parser.add_argument("--output", type=Path, required=True, help="Converted UnitCommitment JSON file")
    parser.add_argument("--time-step-min", type=int, default=15, help="Duration of one source period in minutes. Default: 15")
    return parser.parse_args()


def require_list(data: dict[str, Any], key: str) -> list[Any]:
    value = data.get(key)
    if not isinstance(value, list):
        raise ValueError(f"{key!r} must be a JSON array")
    return value


def as_number(record: dict[str, Any], key: str) -> float:
    value = record.get(key)
    if not isinstance(value, (int, float)):
        raise ValueError(f"{key!r} must be numeric for {record.get('name', record)}")
    return float(value)


def as_integer(record: dict[str, Any], key: str) -> int:
    value = as_number(record, key)
    if not value.is_integer():
        raise ValueError(f"{key!r} must be an integer for {record.get('name', record)}")
    return int(value)


def convert(data: dict[str, Any], time_step_min: int) -> dict[str, Any]:
    if time_step_min <= 0 or 60 % time_step_min != 0:
        raise ValueError("--time-step-min must be a positive divisor of 60")

    buses = require_list(data, "buses")
    generators = require_list(data, "generators")
    branches = require_list(data, "branches")
    load_base = require_list(data, "load_base")
    periods = data.get("n_periods")
    reserve_ratio = data.get("reserve_ratio", 0.0)
    if not isinstance(periods, int) or periods <= 0:
        raise ValueError("n_periods must be a positive integer")
    if periods * time_step_min % 60 != 0:
        raise ValueError("n_periods multiplied by --time-step-min must be a whole number of hours")
    if not isinstance(reserve_ratio, (int, float)) or reserve_ratio < 0:
        raise ValueError("reserve_ratio must be a non-negative number")
    if len(buses) != len(load_base):
        raise ValueError("buses and load_base must have the same length")

    bus_names = {str(bus) for bus in buses}
    if len(bus_names) != len(buses):
        raise ValueError("buses must have unique identifiers")
    horizon = periods * time_step_min
    loads = {str(bus): float(load_base[index]) for index, bus in enumerate(buses)}
    output: dict[str, Any] = {
        "Parameters": {
            "Version": "0.5",
            "Time horizon (min)": horizon,
            "Time step (min)": time_step_min,
            "Base MVA": 100.0,
            "Power balance penalty ($/MW)": 1_000_000.0,
        },
        "Buses": {
            f"b{bus}": {"Load (MW)": [loads[str(bus)]] * periods}
            for bus in buses
        },
        "Generators": {},
        "Branches": {},
        "Reserves": {},
        "Metadata": {
            "Source model": data.get("model_name"),
            "Source periods": periods,
            "Source reserve ratio": float(reserve_ratio),
            "Assumptions": [
                f"Each source period is {time_step_min} minutes.",
                "Generator min_up, min_down, and initial_status are measured in hours.",
                "Branch b is DC susceptance, represented as reactance = 1 / b.",
                "The source has no contingency list; only base-case network constraints are included.",
            ],
        },
    }

    reserve_amount = sum(max(load, 0.0) for load in loads.values()) * float(reserve_ratio)
    if reserve_amount > 0.0:
        output["Reserves"] = {
            "r1": {
                "Type": "Spinning",
                "Amount (MW)": [reserve_amount] * periods,
                "Shortfall penalty ($/MW)": 1_000_000.0,
            }
        }

    output_generators: dict[str, Any] = output["Generators"]
    for generator in generators:
        if not isinstance(generator, dict):
            raise ValueError("generators must contain JSON objects")
        name = generator.get("name")
        bus = generator.get("bus")
        if not isinstance(name, str) or not name:
            raise ValueError("every generator requires a non-empty name")
        if str(bus) not in bus_names:
            raise ValueError(f"generator {name!r} references unknown bus {bus!r}")
        if name in output_generators:
            raise ValueError(f"duplicate generator name: {name}")
        pmin = as_number(generator, "pmin")
        pmax = as_number(generator, "pmax")
        if pmin < 0 or pmax < pmin:
            raise ValueError(f"generator {name!r} has invalid power limits")
        variable_cost = as_number(generator, "var_cost")
        initial_status = as_integer(generator, "initial_status")
        initial_power = as_number(generator, "initial_power")
        if initial_status == 0:
            initial_status = -1
        if initial_status < 0 and initial_power > 1e-3:
            raise ValueError(f"generator {name!r} is initially off but has positive initial power")
        if initial_power > pmax + 1e-6:
            raise ValueError(f"generator {name!r} has initial power above pmax")
        output_generators[name] = {
            "Bus": f"b{bus}",
            "Type": "Thermal",
            "Production cost curve (MW)": [pmin, pmax],
            "Production cost curve ($)": [pmin * variable_cost, pmax * variable_cost],
            "Startup delays (h)": [1],
            "Startup costs ($)": [as_number(generator, "startup_cost")],
            "Shutdown cost ($)": as_number(generator, "shutdown_cost"),
            "Ramp up limit (MW)": as_number(generator, "ramp_up"),
            "Ramp down limit (MW)": as_number(generator, "ramp_down"),
            "Startup limit (MW)": pmax,
            "Shutdown limit (MW)": pmax,
            "Minimum uptime (h)": as_integer(generator, "min_up"),
            "Minimum downtime (h)": as_integer(generator, "min_down"),
            "Initial status (h)": initial_status,
            "Initial power (MW)": initial_power,
            "Reserve eligibility": ["r1"] if reserve_amount > 0.0 else [],
        }

    output_branches: dict[str, Any] = output["Branches"]
    for index, branch in enumerate(branches, start=1):
        if not isinstance(branch, dict):
            raise ValueError("branches must contain JSON objects")
        source = branch.get("from")
        target = branch.get("to")
        if str(source) not in bus_names or str(target) not in bus_names:
            raise ValueError(f"branch {index} references an unknown bus")
        susceptance = as_number(branch, "b")
        flow_limit = as_number(branch, "limit")
        if susceptance == 0.0 or flow_limit <= 0.0:
            raise ValueError(f"branch {index} has invalid susceptance or flow limit")
        output_branches[f"l{index}"] = {
            "Source bus": f"b{source}",
            "Target bus": f"b{target}",
            "Resistance (p.u.)": 0.0,
            "Reactance (p.u.)": 1.0 / susceptance,
            "Normal flow limit (MVA)": flow_limit,
            "Emergency flow limit (MVA)": flow_limit,
            "Flow limit penalty ($/MW)": 1_000_000.0,
        }
    return output


def main() -> int:
    args = parse_args()
    source = json.loads(args.input.read_text(encoding="utf-8"))
    if not isinstance(source, dict):
        raise SystemExit("source JSON must contain an object")
    converted = convert(source, args.time_step_min)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(converted, indent=2) + "\n", encoding="utf-8")
    print(f"Converted SCUC input: {args.output}")
    print(f"Buses: {len(converted['Buses'])}")
    print(f"Generators: {len(converted['Generators'])}")
    print(f"Branches: {len(converted['Branches'])}")
    print(f"Periods: {source['n_periods']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
