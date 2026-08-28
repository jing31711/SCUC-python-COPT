#!/usr/bin/env python3
"""Print thermal commitment and production from a UnitCommitment solution JSON."""
from __future__ import annotations

import argparse
import json
from collections.abc import Mapping, Sequence
from pathlib import Path
from typing import Any


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Inspect thermal time series in a UnitCommitment solution JSON file.")
    parser.add_argument("solution", type=Path, help="Path to solution.json")
    parser.add_argument("--scenario", help="Scenario name for a stochastic solution. Defaults to every scenario.")
    parser.add_argument("--unit", action="append", help="Thermal unit name to print. Pass multiple times.")
    parser.add_argument("--periods", type=int, default=36, help="Maximum number of periods per unit. Default: 36")
    return parser.parse_args()


def scenario_solutions(data: Any) -> Mapping[str, Mapping[str, Any]]:
    if not isinstance(data, Mapping):
        raise ValueError("solution.json must contain a JSON object")
    if "Thermal: Is on" in data:
        return {"s1": data}
    if not all(isinstance(value, Mapping) for value in data.values()):
        raise ValueError("stochastic solution entries must be JSON objects")
    return data


def require_timeseries(section: Mapping[str, Any], key: str) -> Mapping[str, Sequence[Any]]:
    values = section.get(key)
    if not isinstance(values, Mapping):
        raise ValueError(f"{key!r} must be an object keyed by thermal unit name")
    if not all(isinstance(series, Sequence) and not isinstance(series, (str, bytes)) for series in values.values()):
        raise ValueError(f"{key!r} values must be arrays of period values")
    return values


def main() -> int:
    args = parse_args()
    if args.periods <= 0:
        raise SystemExit("--periods must be positive")

    data = json.loads(args.solution.read_text(encoding="utf-8"))
    scenarios = scenario_solutions(data)
    if args.scenario is not None:
        if args.scenario not in scenarios:
            raise SystemExit(f"Scenario not found: {args.scenario}")
        scenarios = {args.scenario: scenarios[args.scenario]}

    requested_units = set(args.unit or [])
    for scenario_name, section in scenarios.items():
        is_on = require_timeseries(section, "Thermal: Is on")
        production = require_timeseries(section, "Thermal: Production (MW)")
        unit_names = list(is_on)
        if requested_units:
            missing = requested_units.difference(unit_names)
            if missing:
                raise SystemExit(f"Thermal unit(s) not found in {scenario_name}: {', '.join(sorted(missing))}")
            unit_names = [name for name in unit_names if name in requested_units]

        print(f"Scenario: {scenario_name}")
        for name in unit_names:
            on_values = list(is_on[name])[: args.periods]
            production_values = list(production.get(name, []))[: args.periods]
            print(f"  {name}")
            print(f"    is_on={on_values}")
            print(f"    production_mw={production_values}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
