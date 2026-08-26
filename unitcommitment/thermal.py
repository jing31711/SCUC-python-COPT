from __future__ import annotations

import math
from typing import Any

from .io import to_scalar, to_timeseries
from .structures import (
    CostSegment,
    Reserve,
    StartupCategory,
    ThermalUnit,
    UnitCommitmentScenario,
)


def read_thermal_json(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    time_multiplier = 60 // scenario["time_step"]
    thermal_units: list[ThermalUnit] = []
    reserves: list[Reserve] = []

    scenario["reserves_by_name"] = {}

    for reserve_name, data in json_data.get("Reserves", {}).items():
        reserve_type = data["Type"].lower()
        if reserve_type == "spinning":
            parsed_type = "spinning"
        elif reserve_type == "non-spinning":
            parsed_type = "non_spinning"
        else:
            continue
        reserve = Reserve(
            name=reserve_name,
            type=parsed_type,
            amount=to_timeseries(data.get("Amount (MW)"), time),
            thermal_units=[],
            shortfall_penalty=to_scalar(data.get("Shortfall penalty ($/MW)"), -1),
        )
        scenario["reserves_by_name"][reserve_name] = reserve
        reserves.append(reserve)

    for reserve_name, data in json_data.get("Reserves", {}).items():
        if reserve_name not in scenario["reserves_by_name"]:
            continue
        parent_name = data.get("Parent")
        if parent_name is None:
            continue
        if parent_name not in scenario["reserves_by_name"]:
            raise ValueError(
                f"Reserve {reserve_name} declares parent {parent_name}, but no reserve with that name exists"
            )
        scenario["reserves_by_name"][reserve_name].parent = scenario["reserves_by_name"][parent_name]

    _validate_no_cycles(reserves)
    _compute_descendants(reserves)

    for unit_name, data in json_data.get("Generators", {}).items():
        unit_type = to_scalar(data.get("Type"))
        if unit_type is None:
            raise ValueError(f"unit {unit_name} has no type specified")
        if unit_type.lower() != "thermal":
            continue

        bus = scenario["bus_by_name"][data["Bus"]]
        curve_mw = [_as_timeseries(value, time) for value in data["Production cost curve (MW)"]]
        curve_cost = [_as_timeseries(value, time) for value in data["Production cost curve ($)"]]
        min_power = curve_mw[0]
        max_power = curve_mw[-1]
        min_power_cost = curve_cost[0]
        cost_segments: list[CostSegment] = []
        for index in range(1, len(curve_mw)):
            mw = [curve_mw[index][t] - curve_mw[index - 1][t] for t in range(time)]
            cost = []
            for t, amount in enumerate(mw):
                value = (curve_cost[index][t] - curve_cost[index - 1][t]) / amount if amount else 0.0
                cost.append(0.0 if math.isnan(value) else value)
            cost_segments.append(CostSegment(mw=mw, cost=cost))

        startup_delays = to_scalar(data.get("Startup delays (h)"), [1])
        startup_costs = to_scalar(data.get("Startup costs ($)"), [0.0])
        startup_categories = [
            StartupCategory(delay=delay * time_multiplier, cost=cost)
            for delay, cost in zip(startup_delays, startup_costs)
        ]

        unit_reserves = [
            scenario["reserves_by_name"][name]
            for name in data.get("Reserve eligibility", [])
            if name in scenario["reserves_by_name"]
        ]

        initial_power = to_scalar(data.get("Initial power (MW)"))
        initial_status = to_scalar(data.get("Initial status (h)"))
        if initial_power is None and initial_status is not None:
            raise ValueError(f"unit {unit_name} has initial status but no initial power")
        if initial_power is not None:
            if initial_status is None:
                raise ValueError(f"unit {unit_name} has initial power but no initial status")
            if initial_status == 0:
                raise ValueError(f"unit {unit_name} has invalid initial status")
            if initial_status < 0 and initial_power > 1e-3:
                raise ValueError(f"unit {unit_name} has invalid initial power")
            initial_status *= time_multiplier

        commitment_status = to_scalar(data.get("Commitment status"), [None for _ in range(time)])
        unit = ThermalUnit(
            name=unit_name,
            bus=bus,
            max_power=max_power,
            min_power=min_power,
            must_run=to_timeseries(data.get("Must run?"), time, default=[False for _ in range(time)]),
            min_power_cost=min_power_cost,
            cost_segments=cost_segments,
            min_uptime=to_scalar(data.get("Minimum uptime (h)"), 1) * time_multiplier,
            min_downtime=to_scalar(data.get("Minimum downtime (h)"), 1) * time_multiplier,
            ramp_up_limit=to_scalar(data.get("Ramp up limit (MW)"), 1e6),
            ramp_down_limit=to_scalar(data.get("Ramp down limit (MW)"), 1e6),
            startup_limit=to_scalar(data.get("Startup limit (MW)"), 1e6),
            shutdown_limit=to_scalar(data.get("Shutdown limit (MW)"), 1e6),
            initial_status=initial_status,
            initial_power=initial_power,
            startup_categories=startup_categories,
            shutdown_cost=to_scalar(data.get("Shutdown cost ($)"), 0.0),
            reserves=unit_reserves,
            non_spinning_capacity=to_scalar(data.get("Non-spinning reserve capacity (MW)"), 0.0),
            commitment_status=commitment_status,
            invest=to_scalar(data.get("Investment cost ($)"), 0.0),
            qmin=to_scalar(data.get("Minimum reactive power (MVAr)"), 0.0),
            qmax=to_scalar(data.get("Maximum reactive power (MVAr)"), 0.0),
        )
        for reserve in unit_reserves:
            reserve.thermal_units.append(unit)
        thermal_units.append(unit)

    scenario["thermal"] = thermal_units
    scenario["thermal_by_name"] = {unit.name: unit for unit in thermal_units}
    scenario["reserves"] = reserves


def _as_timeseries(value: Any, time: int) -> list[float]:
    series = to_timeseries(value, time)
    return [float(item) for item in series]


def _validate_no_cycles(reserves: list[Reserve]) -> None:
    for reserve in reserves:
        visited: set[str] = set()
        current = reserve
        while current is not None:
            if current.name in visited:
                raise ValueError(f"Cycle detected in reserve parent chain involving {current.name}")
            visited.add(current.name)
            current = current.parent


def _compute_descendants(reserves: list[Reserve]) -> None:
    for reserve in reserves:
        for other in reserves:
            if other is reserve:
                continue
            current = other.parent
            while current is not None:
                if current is reserve:
                    reserve.descendants.append(other)
                    break
                current = current.parent


def build_thermal_model(model: Any) -> None:
    _add_thermal_vars(model)
    _add_thermal_objective(model)
    _add_thermal_status_constraints(model)
    _add_thermal_startup_constraints(model)
    _add_thermal_pwl_constraints(model)
    _add_thermal_ramping_constraints(model)
    _add_thermal_startup_shutdown_limit_constraints(model)
    _add_thermal_invest_constraints(model)
    _add_thermal_reserve_constraints(model)
    _add_thermal_non_spinning_reserve_constraints(model)


def _add_thermal_vars(model: Any) -> None:
    for time in range(1, model.instance.time + 1):
        for unit in model.instance.scenarios[0]["thermal"]:
            model.add_binary_var("is_on", (unit.name, time))
            model.add_binary_var("switch_on", (unit.name, time))
            model.add_binary_var("switch_off", (unit.name, time))
            model.store("switch_off")[(unit.name, model.instance.time + 1)] = 0.0
            for index, _category in enumerate(unit.startup_categories, start=1):
                model.add_binary_var("startup", (unit.name, time, index))
        for scenario in model.instance.scenarios:
            for reserve in scenario["reserves"]:
                key = (scenario.name, reserve.name, time)
                if reserve.shortfall_penalty < 0:
                    model.store("reserve_shortfall")[key] = 0.0
                else:
                    model.add_var("reserve_shortfall", key, lb=0.0)
            for unit in scenario["thermal"]:
                prod_key = (scenario.name, unit.name, time)
                model.add_var("prod_above", prod_key, lb=0.0)
                net_injection_key = (scenario.name, unit.bus.name, time)
                model.store("net_injection")[net_injection_key] += model.store("prod_above")[prod_key]
                model.store("net_injection")[net_injection_key] += (
                    unit.min_power[time - 1] * model.store("is_on")[(unit.name, time)]
                )
                for index, segment in enumerate(unit.cost_segments, start=1):
                    model.add_var(
                        "segprod",
                        (scenario.name, unit.name, time, index),
                        lb=0.0,
                        ub=segment.mw[time - 1],
                    )
                for reserve in unit.reserves:
                    model.add_var("reserve", (scenario.name, reserve.name, unit.name, time), lb=0.0)
    for unit in model.instance.scenarios[0]["thermal"]:
        if unit.invest > 0.0:
            model.add_binary_var("invest", unit.name)


def _add_thermal_objective(model: Any) -> None:
    for time in range(1, model.instance.time + 1):
        for unit in model.instance.scenarios[0]["thermal"]:
            model.add_to_objective(
                model.store("is_on")[(unit.name, time)],
                unit.min_power_cost[time - 1],
            )
        for scenario in model.instance.scenarios:
            for unit in scenario["thermal"]:
                for index, segment in enumerate(unit.cost_segments, start=1):
                    model.add_to_objective(
                        model.store("segprod")[(scenario.name, unit.name, time, index)],
                        scenario["probability"] * segment.cost[time - 1],
                    )
            for reserve in scenario["reserves"]:
                if reserve.shortfall_penalty >= 0:
                    model.add_to_objective(
                        model.store("reserve_shortfall")[(scenario.name, reserve.name, time)],
                        reserve.shortfall_penalty * scenario["probability"],
                    )
        for unit in model.instance.scenarios[0]["thermal"]:
            for index, category in enumerate(unit.startup_categories, start=1):
                model.add_to_objective(
                    model.store("startup")[(unit.name, time, index)],
                    category.cost,
                )
            if unit.shutdown_cost > 0:
                model.add_to_objective(
                    model.store("switch_off")[(unit.name, time)],
                    unit.shutdown_cost,
                )
    for unit in model.instance.scenarios[0]["thermal"]:
        if unit.invest > 0.0:
            model.add_to_objective(
                model.store("invest")[unit.name],
                unit.invest * model.instance.scenarios[0]["investment_cost_weight"],
            )


def _add_thermal_status_constraints(model: Any) -> None:
    is_on = model.store("is_on")
    switch_on = model.store("switch_on")
    switch_off = model.store("switch_off")
    for time in range(1, model.instance.time + 1):
        for unit in model.instance.scenarios[0]["thermal"]:
            if unit.must_run[time - 1]:
                model.add_constr("eq_must_run", (unit.name, time), is_on[(unit.name, time)] >= 1.0)
            if unit.commitment_status[time - 1] is not None:
                value = 1.0 if unit.commitment_status[time - 1] else 0.0
                model.add_constr("eq_commitment_status", (unit.name, time), is_on[(unit.name, time)] == value)
            model.add_constr(
                "eq_min_uptime",
                (unit.name, time),
                sum(
                    switch_on[(unit.name, i)]
                    for i in range(time - unit.min_uptime + 1, time + 1)
                    if i >= 1
                ) <= is_on[(unit.name, time)],
            )
            model.add_constr(
                "eq_min_downtime",
                (unit.name, time),
                sum(
                    switch_off[(unit.name, i)]
                    for i in range(time - unit.min_downtime + 1, time + 1)
                    if i >= 1
                ) <= 1 - is_on[(unit.name, time)],
            )
            if time == 1:
                model.add_constr(
                    "eq_binary_link",
                    (unit.name, time),
                    is_on[(unit.name, time)] - _is_initially_on(unit)
                    == switch_on[(unit.name, time)] - switch_off[(unit.name, time)],
                )
                if unit.initial_status is not None and unit.initial_status > 0:
                    expr = sum(
                        switch_off[(unit.name, i)]
                        for i in range(1, unit.min_uptime - unit.initial_status + 1)
                        if i <= model.instance.time
                    )
                    model.add_constr("eq_min_uptime", (unit.name, 0), expr == 0.0)
                elif unit.initial_status is not None:
                    expr = sum(
                        switch_on[(unit.name, i)]
                        for i in range(1, unit.min_downtime + unit.initial_status + 1)
                        if i <= model.instance.time
                    )
                    model.add_constr("eq_min_downtime", (unit.name, 0), expr == 0.0)
            else:
                model.add_constr(
                    "eq_binary_link",
                    (unit.name, time),
                    is_on[(unit.name, time)] - is_on[(unit.name, time - 1)]
                    == switch_on[(unit.name, time)] - switch_off[(unit.name, time)],
                )
            model.add_constr(
                "eq_switch_on_off",
                (unit.name, time),
                switch_on[(unit.name, time)] + switch_off[(unit.name, time)] <= 1.0,
            )


def _add_thermal_startup_constraints(model: Any) -> None:
    startup = model.store("startup")
    switch_on = model.store("switch_on")
    switch_off = model.store("switch_off")
    for time in range(1, model.instance.time + 1):
        for unit in model.instance.scenarios[0]["thermal"]:
            category_count = len(unit.startup_categories)
            model.add_constr(
                "eq_startup_choose",
                (unit.name, time),
                switch_on[(unit.name, time)]
                == sum(startup[(unit.name, time, category)] for category in range(1, category_count + 1)),
            )
            for category in range(1, category_count):
                range_start = time - unit.startup_categories[category].delay + 1
                range_end = time - unit.startup_categories[category - 1].delay
                indices = range(range_start, range_end + 1)
                initial_sum = 1.0 if unit.initial_status is not None and unit.initial_status < 0 and unit.initial_status + 1 in indices else 0.0
                model.add_constr(
                    "eq_startup_restrict",
                    (unit.name, time, category),
                    startup[(unit.name, time, category)]
                    <= initial_sum + sum(switch_off[(unit.name, i)] for i in indices if i >= 1),
                )


def _add_thermal_pwl_constraints(model: Any) -> None:
    is_on = model.store("is_on")
    prod_above = model.store("prod_above")
    segprod = model.store("segprod")
    for scenario in model.instance.scenarios:
        for unit in scenario["thermal"]:
            for time in range(1, model.instance.time + 1):
                reserve = _total_spinning_reserves(model, scenario, unit, time)
                model.add_constr(
                    "eq_prod_limit",
                    (scenario.name, unit.name, time),
                    prod_above[(scenario.name, unit.name, time)] + reserve
                    <= (unit.max_power[time - 1] - unit.min_power[time - 1]) * is_on[(unit.name, time)],
                )
                model.add_constr(
                    "eq_prod_above_def",
                    (scenario.name, unit.name, time),
                    prod_above[(scenario.name, unit.name, time)]
                    == sum(
                        segprod[(scenario.name, unit.name, time, category)]
                        for category in range(1, len(unit.cost_segments) + 1)
                    ),
                )


def _add_thermal_ramping_constraints(model: Any) -> None:
    prod_above = model.store("prod_above")
    for scenario in model.instance.scenarios:
        for unit in scenario["thermal"]:
            if unit.initial_status is not None and unit.initial_status > 0:
                if unit.initial_power is not None and unit.initial_power < unit.min_power[0]:
                    raise ValueError(
                        f"MorLatRam2013.Ramping: Initial power cannot be lower than initial minimum power (generator {unit.name})"
                    )
                reserve = _total_spinning_reserves(model, scenario, unit, 1)
                initial_prod_above = (unit.initial_power or 0.0) - unit.min_power[0]
                model.add_constr(
                    "eq_ramp_up",
                    (scenario.name, unit.name, 1),
                    prod_above[(scenario.name, unit.name, 1)] + reserve - initial_prod_above <= unit.ramp_up_limit,
                )
                model.add_constr(
                    "eq_ramp_down",
                    (scenario.name, unit.name, 1),
                    initial_prod_above - prod_above[(scenario.name, unit.name, 1)] <= unit.ramp_down_limit,
                )
            for time in range(2, model.instance.time + 1):
                if not math.isclose(unit.min_power[time - 1], unit.min_power[time - 2]):
                    raise ValueError(
                        f"MorLatRam2013.Ramping: Time-varying minimum power is not supported (generator {unit.name}, time {time})"
                    )
                reserve = _total_spinning_reserves(model, scenario, unit, time)
                model.add_constr(
                    "eq_ramp_up",
                    (scenario.name, unit.name, time),
                    prod_above[(scenario.name, unit.name, time)]
                    + reserve
                    - prod_above[(scenario.name, unit.name, time - 1)]
                    <= unit.ramp_up_limit,
                )
                model.add_constr(
                    "eq_ramp_down",
                    (scenario.name, unit.name, time),
                    prod_above[(scenario.name, unit.name, time - 1)]
                    - prod_above[(scenario.name, unit.name, time)]
                    <= unit.ramp_down_limit,
                )


def _add_thermal_startup_shutdown_limit_constraints(model: Any) -> None:
    is_on = model.store("is_on")
    prod_above = model.store("prod_above")
    switch_on = model.store("switch_on")
    switch_off = model.store("switch_off")
    for scenario in model.instance.scenarios:
        for unit in scenario["thermal"]:
            for time in range(1, model.instance.time + 1):
                reserve = _total_spinning_reserves(model, scenario, unit, time)
                prod_and_reserve = prod_above[(scenario.name, unit.name, time)] + reserve
                capacity_above_min = unit.max_power[time - 1] - unit.min_power[time - 1]
                startup_headroom = unit.max_power[time - 1] - unit.startup_limit
                shutdown_headroom = unit.max_power[time - 1] - unit.shutdown_limit
                if unit.min_uptime > 1:
                    model.add_constr(
                        "eq_slimit_a",
                        (scenario.name, unit.name, time),
                        prod_and_reserve
                        <= capacity_above_min * is_on[(unit.name, time)]
                        - startup_headroom * switch_on[(unit.name, time)]
                        - shutdown_headroom * switch_off[(unit.name, time + 1)],
                    )
                else:
                    model.add_constr(
                        "eq_slimit_b",
                        (scenario.name, unit.name, time),
                        prod_and_reserve
                        <= capacity_above_min * is_on[(unit.name, time)]
                        - startup_headroom * switch_on[(unit.name, time)],
                    )
                    model.add_constr(
                        "eq_slimit_c",
                        (scenario.name, unit.name, time),
                        prod_and_reserve
                        <= capacity_above_min * is_on[(unit.name, time)]
                        - shutdown_headroom * switch_off[(unit.name, time + 1)],
                    )
            if unit.initial_power is not None and unit.initial_power > unit.shutdown_limit:
                model.add_constr("eq_slimit_init", (scenario.name, unit.name), switch_off[(unit.name, 1)] <= 0.0)


def _add_thermal_invest_constraints(model: Any) -> None:
    is_on = model.store("is_on")
    invest = model.store("invest")
    for unit in model.instance.scenarios[0]["thermal"]:
        if unit.invest <= 0.0:
            continue
        for time in range(1, model.instance.time + 1):
            model.add_constr(
                "eq_invest_link",
                (unit.name, time),
                is_on[(unit.name, time)] <= invest[unit.name],
            )


def _add_thermal_reserve_constraints(model: Any) -> None:
    reserve_store = model.store("reserve")
    reserve_shortfall = model.store("reserve_shortfall")
    for scenario in model.instance.scenarios:
        for reserve in scenario["reserves"]:
            for time in range(1, model.instance.time + 1):
                direct = sum(
                    reserve_store[(scenario.name, reserve.name, unit.name, time)]
                    for unit in reserve.thermal_units
                )
                cascading = sum(
                    reserve_store[(scenario.name, descendant.name, unit.name, time)]
                    for descendant in reserve.descendants
                    for unit in descendant.thermal_units
                )
                model.add_constr(
                    "eq_min_reserve",
                    (scenario.name, reserve.name, time),
                    direct + cascading + reserve_shortfall[(scenario.name, reserve.name, time)]
                    >= reserve.amount[time - 1],
                )


def _add_thermal_non_spinning_reserve_constraints(model: Any) -> None:
    reserve_store = model.store("reserve")
    is_on = model.store("is_on")
    for scenario in model.instance.scenarios:
        for unit in scenario["thermal"]:
            for reserve in unit.reserves:
                if reserve.type != "non_spinning":
                    continue
                for time in range(1, model.instance.time + 1):
                    model.add_constr(
                        "eq_ns_reserve_capacity",
                        (scenario.name, reserve.name, unit.name, time),
                        reserve_store[(scenario.name, reserve.name, unit.name, time)]
                        <= unit.non_spinning_capacity * (1 - is_on[(unit.name, time)]),
                    )


def _total_spinning_reserves(model: Any, scenario: UnitCommitmentScenario, unit: ThermalUnit, time: int) -> Any:
    spinning = [reserve for reserve in unit.reserves if reserve.type == "spinning"]
    if not spinning:
        return 0.0
    return sum(model.store("reserve")[(scenario.name, reserve.name, unit.name, time)] for reserve in spinning)


def _is_initially_on(unit: ThermalUnit) -> float:
    return 1.0 if unit.initial_status is not None and unit.initial_status > 0 else 0.0

