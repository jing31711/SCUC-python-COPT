from __future__ import annotations

from typing import Any


def store_solution(model: Any) -> None:
    solution = {scenario.name: {"Summary": {}} for scenario in model.instance.scenarios}
    _store_bus_solution(solution, model)
    _store_thermal_solution(solution, model)
    _store_profiled_solution(solution, model)
    _store_psload_solution(solution, model)
    _store_virtual_solution(solution, model)
    _store_storage_solution(solution, model)
    _store_transmission_solution(solution, model)
    _store_summary(solution, model)
    model.data["solution"] = solution


def _store_bus_solution(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        buses = _collection(scenario, "bus")
        solution[scenario.name]["Bus: Net injection (MW)"] = _timeseries(model, "ni", scenario, buses)
        solution[scenario.name]["Bus: Load curtail (MW)"] = _timeseries(model, "curtail", scenario, buses)
        solution[scenario.name]["Bus: Reactive net injection (MVAr)"] = _timeseries(model, "qi", scenario, buses)
        solution[scenario.name]["Bus: Reactive load curtail (MVAr)"] = _timeseries(model, "reactive_curtail", scenario, buses)


def _store_thermal_solution(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        thermal = _collection(scenario, "thermal")
        sc_sol = solution[scenario.name]
        sc_sol["Thermal: Is on"] = _timeseries_first_stage(model, "is_on", thermal)
        sc_sol["Thermal: Switch on"] = _timeseries_first_stage(model, "switch_on", thermal)
        sc_sol["Thermal: Switch off"] = _timeseries_first_stage(model, "switch_off", thermal)
        sc_sol["Thermal: Production above minimum (MW)"] = _timeseries(model, "prod_above", scenario, thermal)
        sc_sol["Thermal: Reserve (MW)"] = _thermal_reserve_timeseries(model, scenario)
        sc_sol["Thermal: Production (MW)"] = _thermal_production_timeseries(model, scenario, thermal)
        sc_sol["Thermal: Production cost ($)"] = _thermal_production_cost_timeseries(model, scenario, thermal)
        sc_sol["Thermal: Startup cost ($)"] = _thermal_startup_cost_timeseries(model, thermal)
        sc_sol["Thermal: Shutdown cost ($)"] = _thermal_shutdown_cost_timeseries(model, thermal)


def _store_profiled_solution(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        units = _collection(scenario, "profiled")
        solution[scenario.name]["Profiled: Production (MW)"] = _timeseries(model, "prod", scenario, units)
        solution[scenario.name]["Profiled: Reactive power (MVAr)"] = _timeseries(model, "qg_profiled", scenario, units)


def _store_psload_solution(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        solution[scenario.name]["Price-sensitive load: Demand served (MW)"] = _timeseries(model, "loads", scenario, _collection(scenario, "psload"))


def _store_virtual_solution(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        solution[scenario.name]["Virtual: Cleared (MW)"] = _timeseries(model, "vt_cleared", scenario, _collection(scenario, "virtual"))


def _store_storage_solution(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        storage = _collection(scenario, "storage")
        solution[scenario.name]["Storage: Level (MWh)"] = _timeseries(model, "storage_level", scenario, storage)
        solution[scenario.name]["Storage: Charge rate (MW)"] = _timeseries(model, "charge_rate", scenario, storage)
        solution[scenario.name]["Storage: Discharge rate (MW)"] = _timeseries(model, "discharge_rate", scenario, storage)
        solution[scenario.name]["Storage: Is charging"] = _timeseries(model, "is_charging", scenario, storage)
        solution[scenario.name]["Storage: Is discharging"] = _timeseries(model, "is_discharging", scenario, storage)
        solution[scenario.name]["Storage: Reactive power (MVAr)"] = _timeseries(model, "qs", scenario, storage)


def _store_transmission_solution(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        solution[scenario.name]["Branch: Flow (MW)"] = _timeseries(model, "flow", scenario, _collection(scenario, "branches"))
        solution[scenario.name]["Branch: Overflow (MW)"] = _timeseries(model, "overflow", scenario, _collection(scenario, "branches"))
        solution[scenario.name]["Interface: Flow (MW)"] = _timeseries(model, "interface_flow", scenario, _collection(scenario, "interfaces"))
        solution[scenario.name]["Interface: Overflow (MW)"] = _timeseries(model, "interface_overflow", scenario, _collection(scenario, "interfaces"))


def _store_summary(solution: dict[str, Any], model: Any) -> None:
    for scenario in model.instance.scenarios:
        summary = solution[scenario.name]["Summary"]
        summary["Solver: Objective value ($)"] = model.objective_value
        summary["Solver: Termination status"] = model.termination_status
        summary["Solver: Solve time (s)"] = model.solve_time
        summary["Solver: Optimality gap (%)"] = model.relative_gap
        summary["Solver: Objective bound"] = model.objective_bound
        curtail = solution[scenario.name]["Bus: Load curtail (MW)"]
        summary["Bus: Total load curtailment (MW)"] = round(sum(sum(values) for values in curtail.values()), 5)


def _timeseries(model: Any, store: str, scenario: Any, collection: list[Any]) -> dict[str, list[float]]:
    values = _store_values(model, store)
    return {
        item.name: [round(model.value(values[(scenario.name, item.name, time)]), 5) for time in _times(model)]
        for item in collection
        if _has_all(values, [(scenario.name, item.name, time) for time in _times(model)])
    }


def _timeseries_first_stage(model: Any, store: str, collection: list[Any]) -> dict[str, list[float]]:
    values = _store_values(model, store)
    return {
        item.name: [round(model.value(values[(item.name, time)]), 5) for time in _times(model)]
        for item in collection
        if _has_all(values, [(item.name, time) for time in _times(model)])
    }


def _times(model: Any) -> range:
    return range(1, model.instance.time + 1)


def _collection(scenario: Any, key: str) -> list[Any]:
    return scenario.get(key, [])


def _store_values(model: Any, store: str) -> dict[Any, Any]:
    return model.variables.get(store, {})


def _has_all(values: dict[Any, Any], keys: list[Any]) -> bool:
    return all(key in values for key in keys)


def _thermal_reserve_timeseries(model: Any, scenario: Any) -> dict[str, dict[str, list[float]]]:
    values = _store_values(model, "reserve")
    result: dict[str, dict[str, list[float]]] = {}
    for reserve in _collection(scenario, "reserves"):
        unit_values = {}
        for unit in reserve.thermal_units:
            keys = [(scenario.name, reserve.name, unit.name, time) for time in _times(model)]
            if _has_all(values, keys):
                unit_values[unit.name] = [round(model.value(values[key]), 5) for key in keys]
        result[reserve.name] = unit_values
    return result


def _thermal_production_timeseries(model: Any, scenario: Any, thermal: list[Any]) -> dict[str, list[float]]:
    prod_above = _store_values(model, "prod_above")
    is_on = _store_values(model, "is_on")
    result = {}
    for unit in thermal:
        prod_keys = [(scenario.name, unit.name, time) for time in _times(model)]
        status_keys = [(unit.name, time) for time in _times(model)]
        if not _has_all(prod_above, prod_keys) or not _has_all(is_on, status_keys):
            continue
        result[unit.name] = [
            round(model.value(prod_above[prod_key]) + unit.min_power[time - 1] * model.value(is_on[status_key]), 5)
            for time, prod_key, status_key in zip(_times(model), prod_keys, status_keys)
        ]
    return result


def _thermal_production_cost_timeseries(model: Any, scenario: Any, thermal: list[Any]) -> dict[str, list[float]]:
    segprod = _store_values(model, "segprod")
    is_on = _store_values(model, "is_on")
    result = {}
    for unit in thermal:
        status_keys = [(unit.name, time) for time in _times(model)]
        segprod_keys = [
            [(scenario.name, unit.name, time, index) for index, _segment in enumerate(unit.cost_segments, start=1)]
            for time in _times(model)
        ]
        if not _has_all(is_on, status_keys) or any(not _has_all(segprod, keys) for keys in segprod_keys):
            continue
        result[unit.name] = [
            round(
                unit.min_power_cost[time - 1] * model.value(is_on[status_key])
                + sum(
                    segment.cost[time - 1] * model.value(segprod[key])
                    for segment, key in zip(unit.cost_segments, keys)
                ),
                5,
            )
            for time, status_key, keys in zip(_times(model), status_keys, segprod_keys)
        ]
    return result


def _thermal_startup_cost_timeseries(model: Any, thermal: list[Any]) -> dict[str, list[float]]:
    startup = _store_values(model, "startup")
    result = {}
    for unit in thermal:
        startup_keys = [
            [(unit.name, time, index) for index, _category in enumerate(unit.startup_categories, start=1)]
            for time in _times(model)
        ]
        if any(not _has_all(startup, keys) for keys in startup_keys):
            continue
        result[unit.name] = [
            round(
                sum(category.cost * model.value(startup[key]) for category, key in zip(unit.startup_categories, keys)),
                5,
            )
            for keys in startup_keys
        ]
    return result


def _thermal_shutdown_cost_timeseries(model: Any, thermal: list[Any]) -> dict[str, list[float]]:
    switch_off = _store_values(model, "switch_off")
    result = {}
    for unit in thermal:
        keys = [(unit.name, time) for time in _times(model)]
        if _has_all(switch_off, keys):
            result[unit.name] = [round(unit.shutdown_cost * model.value(switch_off[key]), 5) for key in keys]
    return result


