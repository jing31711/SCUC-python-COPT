from __future__ import annotations

from typing import Any

from .io import to_scalar, to_timeseries
from .structures import (
    Interface,
    PriceSensitiveLoad,
    ProfiledUnit,
    StorageUnit,
    UnitCommitmentScenario,
    VirtualTransaction,
)


def read_profiled_json(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    profiled_units: list[ProfiledUnit] = []
    for unit_name, data in json_data.get("Generators", {}).items():
        unit_type = data.get("Type")
        if unit_type is None or unit_type.lower() != "profiled":
            continue
        unit = ProfiledUnit(
            name=unit_name,
            bus=scenario["bus_by_name"][data["Bus"]],
            min_power=to_timeseries(data.get("Minimum power (MW)", 0.0), time),
            max_power=to_timeseries(data.get("Maximum power (MW)"), time),
            cost=to_timeseries(data.get("Cost ($/MW)"), time),
            invest=to_scalar(data.get("Investment cost ($)"), 0.0),
            qmin=to_scalar(data.get("Minimum reactive power (MVAr)"), 0.0),
            qmax=to_scalar(data.get("Maximum reactive power (MVAr)"), 0.0),
        )
        profiled_units.append(unit)
    scenario["profiled"] = profiled_units
    scenario["profiled_by_name"] = {unit.name: unit for unit in profiled_units}


def read_psload_json(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    loads: list[PriceSensitiveLoad] = []
    for load_name, data in json_data.get("Price-sensitive loads", {}).items():
        load = PriceSensitiveLoad(
            name=load_name,
            bus=scenario["bus_by_name"][data["Bus"]],
            demand=to_timeseries(data.get("Demand (MW)"), time),
            revenue=to_timeseries(data.get("Revenue ($/MW)"), time),
        )
        loads.append(load)
    scenario["psload"] = loads
    scenario["psload_by_name"] = {load.name: load for load in loads}


def read_virtual_json(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    virtuals: list[VirtualTransaction] = []
    for name, data in json_data.get("Virtual transactions", {}).items():
        vt_type = data["Type"].lower()
        if vt_type not in {"inc", "dec", "utc"}:
            raise ValueError(f"Unknown virtual transaction type {data['Type']} for virtual transaction {name}. Expected INC, DEC, or UTC.")
        if vt_type == "utc":
            bus_source = scenario["bus_by_name"][data["Source bus"]]
            bus_sink = scenario["bus_by_name"][data["Sink bus"]]
        else:
            bus_source = scenario["bus_by_name"][data["Bus"]]
            bus_sink = bus_source
        price_key = "Offer price ($/MW)" if vt_type == "inc" else "Bid price ($/MW)"
        virtuals.append(
            VirtualTransaction(
                name=name,
                type=vt_type,
                bus_source=bus_source,
                bus_sink=bus_sink,
                price=to_timeseries(data.get(price_key), time),
                max_quantity=to_timeseries(data.get("Maximum quantity (MW)"), time),
            )
        )
    scenario["virtual"] = virtuals
    scenario["virtual_by_name"] = {virtual.name: virtual for virtual in virtuals}


def read_storage_json(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    units: list[StorageUnit] = []
    for name, data in json_data.get("Storage units", {}).items():
        min_level = to_timeseries(to_scalar(data.get("Minimum level (MWh)"), 0.0), time)
        max_level = to_timeseries(data.get("Maximum level (MWh)"), time)
        units.append(
            StorageUnit(
                name=name,
                bus=scenario["bus_by_name"][data["Bus"]],
                min_level=min_level,
                max_level=max_level,
                simultaneous_charge_and_discharge=to_timeseries(
                    to_scalar(data.get("Allow simultaneous charging and discharging"), True), time
                ),
                charge_cost=to_timeseries(data.get("Charge cost ($/MW)"), time),
                discharge_cost=to_timeseries(data.get("Discharge cost ($/MW)"), time),
                charge_efficiency=to_timeseries(to_scalar(data.get("Charge efficiency"), 1.0), time),
                discharge_efficiency=to_timeseries(to_scalar(data.get("Discharge efficiency"), 1.0), time),
                loss_factor=to_timeseries(to_scalar(data.get("Loss factor"), 0.0), time),
                min_charge_rate=to_timeseries(to_scalar(data.get("Minimum charge rate (MW)"), 0.0), time),
                max_charge_rate=to_timeseries(data.get("Maximum charge rate (MW)"), time),
                min_discharge_rate=to_timeseries(to_scalar(data.get("Minimum discharge rate (MW)"), 0.0), time),
                max_discharge_rate=to_timeseries(data.get("Maximum discharge rate (MW)"), time),
                initial_level=to_scalar(data.get("Initial level (MWh)"), 0.0),
                min_ending_level=to_scalar(data.get("Last period minimum level (MWh)"), min_level[-1]),
                max_ending_level=to_scalar(data.get("Last period maximum level (MWh)"), max_level[-1]),
                invest=to_scalar(data.get("Investment cost ($)"), 0.0),
                qmin=to_scalar(data.get("Minimum reactive power (MVAr)"), 0.0),
                qmax=to_scalar(data.get("Maximum reactive power (MVAr)"), 0.0),
                apparent_power_limit=to_scalar(data.get("Apparent power limit (MVA)"), float("inf")),
            )
        )
    scenario["storage"] = units
    scenario["storage_by_name"] = {unit.name: unit for unit in units}


def read_interface_json(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    interfaces: list[Interface] = []
    for name, data in json_data.get("Interfaces", {}).items():
        branches = []
        weights: dict[str, float] = {}
        for branch_name, weight in (data.get("Branches") or {}).items():
            branch = scenario["branch_by_name"][branch_name]
            branches.append(branch)
            weights[branch_name] = float(weight)
        interfaces.append(
            Interface(
                name=name,
                offset=len(interfaces) + 1,
                branches=branches,
                weight_by_branch=weights,
                net_flow_ub=to_timeseries(to_scalar(data.get("Net flow upper limit (MW)"), float("inf")), time),
                net_flow_lb=to_timeseries(to_scalar(data.get("Net flow lower limit (MW)"), float("-inf")), time),
                flow_limit_penalty=to_timeseries(to_scalar(data.get("Flow limit penalty ($/MW)"), 5000.0), time),
            )
        )
    scenario["interfaces"] = interfaces
    scenario["interface_by_name"] = {interface.name: interface for interface in interfaces}


def build_profiled_model(model: Any) -> None:
    for scenario in model.instance.scenarios:
        for unit in scenario["profiled"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, unit.name, time)
                prod = model.add_var("prod", key, lb=unit.min_power[time - 1], ub=unit.max_power[time - 1])
                qg = model.add_var("qg_profiled", key, lb=unit.qmin, ub=unit.qmax)
                model.store("net_injection")[(scenario.name, unit.bus.name, time)] += prod
                model.store("net_reactive_injection")[(scenario.name, unit.bus.name, time)] += qg
                model.add_to_objective(prod, unit.cost[time - 1] * scenario["probability"])
    for unit in model.instance.scenarios[0]["profiled"]:
        if unit.invest > 0.0:
            invest = model.add_binary_var("invest", unit.name)
            model.add_to_objective(invest, unit.invest * model.instance.scenarios[0]["investment_cost_weight"])
            for scenario in model.instance.scenarios:
                scenario_unit = scenario["profiled_by_name"][unit.name]
                for time in range(1, model.instance.time + 1):
                    prod = model.store("prod")[(scenario.name, scenario_unit.name, time)]
                    model.add_constr("eq_invest_prod_ub", (scenario.name, scenario_unit.name, time), prod <= scenario_unit.max_power[time - 1] * invest)
                    model.add_constr("eq_invest_prod_lb", (scenario.name, scenario_unit.name, time), prod >= scenario_unit.min_power[time - 1] * invest)


def build_psload_model(model: Any) -> None:
    for scenario in model.instance.scenarios:
        for load in scenario["psload"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, load.name, time)
                served = model.add_var("loads", key, lb=0.0, ub=load.demand[time - 1])
                model.store("net_injection")[(scenario.name, load.bus.name, time)] -= served
                model.add_to_objective(served, -load.revenue[time - 1] * scenario["probability"])


def build_virtual_model(model: Any) -> None:
    for scenario in model.instance.scenarios:
        for virtual in scenario["virtual"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, virtual.name, time)
                cleared = model.add_var("vt_cleared", key, lb=0.0, ub=virtual.max_quantity[time - 1])
                if virtual.type == "inc":
                    model.store("net_injection")[(scenario.name, virtual.bus_source.name, time)] += cleared
                    model.add_to_objective(cleared, virtual.price[time - 1] * scenario["probability"])
                elif virtual.type == "dec":
                    model.store("net_injection")[(scenario.name, virtual.bus_sink.name, time)] -= cleared
                    model.add_to_objective(cleared, -virtual.price[time - 1] * scenario["probability"])
                else:
                    model.store("net_injection")[(scenario.name, virtual.bus_source.name, time)] += cleared
                    model.store("net_injection")[(scenario.name, virtual.bus_sink.name, time)] -= cleared
                    model.add_to_objective(cleared, -virtual.price[time - 1] * scenario["probability"])


def build_storage_model(model: Any) -> None:
    for scenario in model.instance.scenarios:
        for unit in scenario["storage"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, unit.name, time)
                level = model.add_var("storage_level", key, lb=0.0 if unit.invest > 0.0 else unit.min_level[time - 1], ub=unit.max_level[time - 1])
                charge = model.add_var("charge_rate", key, ub=model.infinity)
                discharge = model.add_var("discharge_rate", key, ub=model.infinity)
                is_charging = model.add_binary_var("is_charging", key)
                is_discharging = model.add_binary_var("is_discharging", key)
                qs = model.add_var("qs", key, lb=unit.qmin, ub=unit.qmax)
                model.store("net_injection")[(scenario.name, unit.bus.name, time)] += discharge - charge
                model.store("net_reactive_injection")[(scenario.name, unit.bus.name, time)] += qs
                model.add_to_objective(charge, unit.charge_cost[time - 1] * scenario["probability"])
                model.add_to_objective(discharge, unit.discharge_cost[time - 1] * scenario["probability"])
                if not unit.simultaneous_charge_and_discharge[time - 1]:
                    model.add_constr("eq_simultaneous_charge_and_discharge", key, is_charging + is_discharging <= 1.0)
                model.add_constr("eq_min_charge_rate", key, charge >= is_charging * unit.min_charge_rate[time - 1])
                model.add_constr("eq_max_charge_rate", key, charge <= is_charging * unit.max_charge_rate[time - 1])
                model.add_constr("eq_min_discharge_rate", key, discharge >= is_discharging * unit.min_discharge_rate[time - 1])
                model.add_constr("eq_max_discharge_rate", key, discharge <= is_discharging * unit.max_discharge_rate[time - 1])
                prev_level = unit.initial_level if time == 1 else model.store("storage_level")[(scenario.name, unit.name, time - 1)]
                time_step = scenario["time_step"] / 60
                model.add_constr(
                    "eq_storage_transition",
                    key,
                    level
                    == (1 - unit.loss_factor[time - 1]) * prev_level
                    + charge * time_step * unit.charge_efficiency[time - 1]
                    - discharge * time_step / unit.discharge_efficiency[time - 1],
                )
                if time == model.instance.time:
                    model.add_constr("eq_ending_level", (scenario.name, unit.name), level >= unit.min_ending_level)
                    model.add_constr("eq_ending_level_ub", (scenario.name, unit.name), level <= unit.max_ending_level)
    for unit in model.instance.scenarios[0]["storage"]:
        if unit.invest > 0.0:
            invest = model.add_binary_var("invest_storage", unit.name)
            model.add_to_objective(invest, unit.invest * model.instance.scenarios[0]["investment_cost_weight"])
            for scenario in model.instance.scenarios:
                scenario_unit = scenario["storage_by_name"][unit.name]
                for time in range(1, model.instance.time + 1):
                    level = model.store("storage_level")[(scenario.name, scenario_unit.name, time)]
                    model.add_constr("eq_invest_storage_level_ub", (scenario.name, scenario_unit.name, time), level <= scenario_unit.max_level[time - 1] * invest)
                    model.add_constr("eq_invest_storage_level_lb", (scenario.name, scenario_unit.name, time), level >= scenario_unit.min_level[time - 1] * invest)


def build_interface_model(model: Any) -> None:
    for scenario in model.instance.scenarios:
        for interface in scenario["interfaces"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, interface.name, time)
                flow = model.add_var("interface_flow", key, lb=-model.infinity, ub=model.infinity)
                if interface.flow_limit_penalty[time - 1] < 0:
                    overflow = model.store("interface_overflow")[key] = 0.0
                else:
                    overflow = model.add_var("interface_overflow", key, lb=0.0)
                    model.add_to_objective(overflow, interface.flow_limit_penalty[time - 1] * scenario["probability"])
                expr = 0.0
                for branch in interface.branches:
                    branch_key = (scenario.name, branch.name, time)
                    if branch_key in model.store("flow"):
                        expr += interface.weight_by_branch[branch.name] * model.store("flow")[branch_key]
                model.add_constr("eq_interface_flow_def", key, flow == expr)
                if interface.net_flow_ub[time - 1] != float("inf"):
                    model.add_constr("eq_interface_flow_ub", key, flow <= interface.net_flow_ub[time - 1] + overflow)
                if interface.net_flow_lb[time - 1] != float("-inf"):
                    model.add_constr("eq_interface_flow_lb", key, flow >= interface.net_flow_lb[time - 1] - overflow)
