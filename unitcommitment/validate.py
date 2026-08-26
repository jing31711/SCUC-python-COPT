from __future__ import annotations

from typing import Any


def validate(instance: Any, solution: dict[str, Any], tol: float = 0.01) -> bool:
    errors = 0
    errors += _validate_profiled(instance, solution, tol)
    errors += _validate_psload(instance, solution, tol)
    errors += _validate_virtual(instance, solution, tol)
    return errors == 0


def _validate_profiled(instance: Any, solution: dict[str, Any], tol: float) -> int:
    errors = 0
    for scenario in instance.scenarios:
        production = solution[scenario.name].get("Profiled: Production (MW)", {})
        for unit in scenario["profiled"]:
            for time, value in enumerate(production.get(unit.name, [])):
                if value < unit.min_power[time] - tol or value > unit.max_power[time] + tol:
                    errors += 1
    return errors


def _validate_psload(instance: Any, solution: dict[str, Any], tol: float) -> int:
    errors = 0
    for scenario in instance.scenarios:
        served = solution[scenario.name].get("Price-sensitive load: Demand served (MW)", {})
        for load in scenario["psload"]:
            for time, value in enumerate(served.get(load.name, [])):
                if value < -tol or value > load.demand[time] + tol:
                    errors += 1
    return errors


def _validate_virtual(instance: Any, solution: dict[str, Any], tol: float) -> int:
    errors = 0
    for scenario in instance.scenarios:
        cleared = solution[scenario.name].get("Virtual: Cleared (MW)", {})
        for virtual in scenario["virtual"]:
            for time, value in enumerate(cleared.get(virtual.name, [])):
                if value < -tol or value > virtual.max_quantity[time] + tol:
                    errors += 1
    return errors
