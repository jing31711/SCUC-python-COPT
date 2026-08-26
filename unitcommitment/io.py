from __future__ import annotations

import gzip
import json
from collections import OrderedDict
from pathlib import Path
from typing import Any, Iterable

from .migrate import migrate
from .structures import (
    DEFAULT_EXTENSIONS,
    Bus,
    Extension,
    UnitCommitmentInstance,
    UnitCommitmentScenario,
)


def to_scalar(value: Any, default: Any = None) -> Any:
    return default if value is None else value


def to_timeseries(value: Any, time: int, default: Any = None) -> list[Any]:
    if value is None:
        return default
    if isinstance(value, list):
        return value
    return [value for _ in range(time)]


def read(
    path: str | Path | Iterable[str | Path],
    extensions: list[Extension] | None = None,
    repair: bool = True,
) -> UnitCommitmentInstance:
    merged_extensions = _merge_extensions(extensions or [])
    if isinstance(path, (str, Path)):
        scenario = _read_scenario(Path(path), merged_extensions)
        scenario.name = "s1"
        scenario["probability"] = 1.0
        instance = UnitCommitmentInstance(
            time=scenario["time"],
            scenarios=[scenario],
            extensions=merged_extensions,
            extension_by_slot=_build_extension_by_slot(merged_extensions),
        )
    else:
        paths = [Path(p) for p in path]
        scenarios = [_read_scenario(p, merged_extensions) for p in paths]
        _assign_scenario_names(scenarios, paths)
        _normalize_probabilities(scenarios)
        instance = UnitCommitmentInstance(
            time=scenarios[0]["time"],
            scenarios=scenarios,
            extensions=merged_extensions,
            extension_by_slot=_build_extension_by_slot(merged_extensions),
        )
    if repair:
        _repair(instance)
    return instance


def write(path: str | Path, solution: dict[str, Any]) -> None:
    with Path(path).open("w", encoding="utf-8") as file:
        json.dump(solution, file, indent=2)


def _parse_json_file(path: Path) -> OrderedDict[str, Any]:
    opener = gzip.open if path.suffix == ".gz" else open
    with opener(path, "rt", encoding="utf-8") as file:
        return json.load(file, object_pairs_hook=OrderedDict)


def _read_scenario(path: Path, extensions: list[Extension]) -> UnitCommitmentScenario:
    json_data = _parse_json_file(path)
    migrate(json_data)
    params = json_data["Parameters"]
    time = _parse_time(params)
    scenario = UnitCommitmentScenario(name=to_scalar(params.get("Scenario name"), ""))
    scenario["time"] = time
    scenario["time_step"] = to_scalar(params.get("Time step (min)"), 60)
    scenario["probability"] = to_scalar(params.get("Scenario weight"), 1.0)
    scenario["investment_cost_weight"] = to_scalar(params.get("Investment cost weight"), 1.0)
    scenario["power_balance_penalty"] = to_timeseries(
        params.get("Power balance penalty ($/MW)"),
        time,
        default=[1000.0 for _ in range(time)],
    )
    scenario["base_mva"] = to_scalar(params.get("Base MVA"), 100.0)
    scenario["raw_json"] = json_data
    _read_buses(json_data, scenario)
    for extension in extensions:
        extension.read_json(json_data, scenario)
    return scenario


def _parse_time(params: dict[str, Any]) -> int:
    time_horizon = params.get("Time horizon (min)")
    if time_horizon is None:
        time_horizon = params.get("Time (h)", params.get("Time horizon (h)"))
        if time_horizon is not None:
            time_horizon *= 60
    if time_horizon is None:
        raise ValueError("Missing parameter: Time horizon (min)")
    if int(time_horizon) != time_horizon:
        raise ValueError("Time horizon must be an integer in minutes")
    time_horizon = int(time_horizon)
    time_step = to_scalar(params.get("Time step (min)"), 60)
    if 60 % time_step != 0:
        raise ValueError(f"Time step {time_step} is not a divisor of 60")
    if time_horizon % time_step != 0:
        raise ValueError(f"Time step {time_step} is not a divisor of time horizon {time_horizon}")
    return time_horizon // time_step


def _read_buses(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    buses: list[Bus] = []
    buses_by_name: dict[str, Bus] = {}
    for bus_name, data in json_data["Buses"].items():
        bus = Bus(
            name=bus_name,
            offset=len(buses),
            load=to_timeseries(data.get("Load (MW)"), time),
            reactive_load=to_timeseries(data.get("Load (MVAr)"), time, default=[0.0 for _ in range(time)]),
            vmin=to_scalar(data.get("Minimum voltage (p.u.)"), float("-inf")),
            vmax=to_scalar(data.get("Maximum voltage (p.u.)"), float("inf")),
            bus_type=to_scalar(data.get("Bus type"), "PQ"),
        )
        buses_by_name[bus_name] = bus
        buses.append(bus)
    scenario["bus"] = buses
    scenario["bus_by_name"] = buses_by_name


def _merge_extensions(user_extensions: list[Extension]) -> list[Extension]:
    merged = list(DEFAULT_EXTENSIONS)
    for extension in user_extensions:
        slot = extension.slot
        index = next(
            (i for i, default in enumerate(merged) if default.slot == slot),
            None,
        )
        if index is None:
            merged.append(extension)
        else:
            merged[index] = extension
    return merged


def _build_extension_by_slot(extensions: list[Extension]) -> dict[str, Extension]:
    return {extension.slot: extension for extension in extensions if extension.slot is not None}


def _assign_scenario_names(scenarios: list[UnitCommitmentScenario], paths: list[Path]) -> None:
    counts: dict[str, int] = {}
    for scenario, path in zip(scenarios, paths):
        base = scenario.name or path.name.split(".")[0]
        count = counts.get(base, 0)
        counts[base] = count + 1
        scenario.name = base if count == 0 else f"{base}_{count}"


def _normalize_probabilities(scenarios: list[UnitCommitmentScenario]) -> None:
    total = sum(scenario["probability"] for scenario in scenarios)
    for scenario in scenarios:
        scenario["probability"] /= total


def _repair(instance: UnitCommitmentInstance) -> None:
    issues = _find_repair_issues(instance)
    if issues:
        issue_list = "; ".join(issues[:3])
        if len(issues) > 3:
            issue_list += f"; and {len(issues) - 3} more"
        raise NotImplementedError(
            "Automatic repair is not implemented in the Python/COPT port. "
            f"Load this instance with repair=False after fixing: {issue_list}."
        )


def _find_repair_issues(instance: UnitCommitmentInstance) -> list[str]:
    issues: list[str] = []
    for scenario in instance.scenarios:
        for unit in scenario.get("thermal", []):
            for index in range(1, len(unit.startup_categories)):
                current = unit.startup_categories[index]
                previous = unit.startup_categories[index - 1]
                if current.delay <= previous.delay:
                    issues.append(f"generator {unit.name} has non-increasing startup delays")
                if current.cost < previous.cost:
                    issues.append(f"generator {unit.name} has decreasing startup costs")
            if unit.initial_status is not None and unit.initial_status > 0 and unit.initial_power is not None:
                if unit.initial_power < unit.min_power[0]:
                    issues.append(f"generator {unit.name} has initial power below minimum power")
            for time, min_power in enumerate(unit.min_power, start=1):
                if unit.startup_limit < min_power:
                    issues.append(f"generator {unit.name} has startup limit below minimum power at time {time}")
            for time, costs in enumerate(zip(*(segment.cost for segment in unit.cost_segments)), start=1):
                for index in range(1, len(costs)):
                    if costs[index] < costs[index - 1] - 1e-5:
                        issues.append(f"generator {unit.name} has non-convex production cost curve at time {time}")
    return issues
