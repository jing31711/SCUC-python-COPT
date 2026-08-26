from __future__ import annotations

from typing import Any

import numpy as np

from .io import to_scalar, to_timeseries
from .structures import Branch, Contingency, ShuntDevice, UnitCommitmentScenario


def read_transmission_json(json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
    time = scenario["time"]
    branches: list[Branch] = []
    for name, data in json_data.get("Branches", {}).items():
        resistance = to_scalar(data.get("Resistance (p.u.)"), 0.0)
        reactance = to_scalar(data.get("Reactance (p.u.)"), 0.0)
        susceptance = reactance / (resistance**2 + reactance**2) if resistance**2 + reactance**2 else 0.0
        branches.append(
            Branch(
                name=name,
                offset=len(branches) + 1,
                source=scenario["bus_by_name"][data["Source bus"]],
                target=scenario["bus_by_name"][data["Target bus"]],
                resistance=resistance,
                reactance=reactance,
                susceptance=susceptance,
                shunt_conductance=to_scalar(data.get("Shunt conductance (p.u.)"), 0.0),
                shunt_susceptance=to_scalar(data.get("Shunt susceptance (p.u.)"), 0.0),
                tap_ratio=to_scalar(data.get("Tap ratio (p.u.)"), 1.0),
                phase_shift=to_scalar(data.get("Phase shift (rad)"), 0.0),
                normal_flow_limit=to_timeseries(data.get("Normal flow limit (MVA)"), time, default=[1e8 for _ in range(time)]),
                emergency_flow_limit=to_timeseries(data.get("Emergency flow limit (MVA)"), time, default=[1e8 for _ in range(time)]),
                flow_limit_penalty=to_timeseries(data.get("Flow limit penalty ($/MW)"), time, default=[5000.0 for _ in range(time)]),
                angle_diff_min=to_scalar(data.get("Minimum angle difference (rad)"), float("-inf")),
                angle_diff_max=to_scalar(data.get("Maximum angle difference (rad)"), float("inf")),
                invest=to_scalar(data.get("Investment cost ($)"), 0.0),
                max_copy=to_scalar(data.get("Maximum parallel circuits"), 1),
            )
        )
    scenario["branches"] = branches
    scenario["branch_by_name"] = {branch.name: branch for branch in branches}

    contingencies: list[Contingency] = []
    for name, data in json_data.get("Contingencies", {}).items():
        if data.get("Affected units") is not None:
            raise ValueError("Unit contingencies are not currently supported")
        affected = [scenario["branch_by_name"][branch] for branch in data.get("Affected branches", [])]
        contingencies.append(Contingency(name=name, branches=affected))
    scenario["contingencies"] = contingencies
    scenario["contingencies_by_name"] = {contingency.name: contingency for contingency in contingencies}

    shunts: list[ShuntDevice] = []
    for name, data in json_data.get("Shunt devices", {}).items():
        shunts.append(
            ShuntDevice(
                name=name,
                bus=scenario["bus_by_name"][data["Bus"]],
                conductance=to_scalar(data.get("Conductance (p.u.)"), 0.0),
                susceptance=to_scalar(data.get("Susceptance (p.u.)"), 0.0),
                status=to_timeseries(data.get("Status"), time, default=[True for _ in range(time)]),
            )
        )
    scenario["shunts"] = shunts
    scenario["shunt_by_name"] = {shunt.name: shunt for shunt in shunts}
    scenario["shunts_by_bus"] = {bus.name: [shunt for shunt in shunts if shunt.bus is bus] for bus in scenario["bus"]}
    scenario["branches_by_source_bus"] = {bus.name: [branch for branch in branches if branch.source is bus] for bus in scenario["bus"]}
    scenario["branches_by_target_bus"] = {bus.name: [branch for branch in branches if branch.target is bus] for bus in scenario["bus"]}


def build_copperplate_model(model: Any) -> None:
    ni = model.store("ni")
    for scenario in model.instance.scenarios:
        for time in range(1, model.instance.time + 1):
            total_shunt_loss = sum(
                shunt.conductance * scenario["base_mva"]
                for shunt in scenario["shunts"]
                if shunt.status[time - 1]
            )
            model.add_constr(
                "eq_power_balance",
                (scenario.name, time),
                sum(ni[(scenario.name, bus.name, time)] for bus in scenario["bus"]) == total_shunt_loss,
            )


def build_shift_factors_model(model: Any, ext: Any) -> None:
    if ext.lazy:
        raise NotImplementedError("lazy shift-factor transmission is not implemented in the Python/COPT port yet.")
    _check_shift_factors_model(model)
    for scenario in model.instance.scenarios:
        _ensure_shift_factor_data(scenario, ext.isf_cutoff, ext.lodf_cutoff)
    _add_dc_overflow_and_flow_vars(model)
    _add_shift_factor_flow_constraints(model)
    build_copperplate_model(model)


def build_simplified_transmission_model(model: Any, ext: Any) -> None:
    if ext.lazy:
        raise NotImplementedError("lazy simplified transmission is not implemented in the Python/COPT port yet.")
    _add_dc_overflow_and_flow_vars(model)
    _add_simplified_dc_flow_constraints(model)
    build_copperplate_model(model)


def compute_shift_factor_data(scenario: UnitCommitmentScenario, isf_cutoff: float = 0.005, lodf_cutoff: float = 0.001) -> None:
    if len(scenario["branches"]) == 0:
        return
    isf = injection_shift_factors(buses=scenario["bus"], branches=scenario["branches"])
    lodf = line_outage_factors(buses=scenario["bus"], branches=scenario["branches"], isf=isf)
    isf[np.abs(isf) < isf_cutoff] = 0.0
    lodf[np.abs(lodf) < lodf_cutoff] = 0.0
    scenario["isf"] = isf
    scenario["lodf"] = lodf


def injection_shift_factors(*, buses: list[Any], branches: list[Branch]) -> np.ndarray:
    incidence = _reduced_incidence_matrix(buses=buses, branches=branches)
    susceptance = np.diag([branch.susceptance for branch in branches])
    laplacian = incidence.T @ susceptance @ incidence
    return susceptance @ incidence @ np.linalg.inv(laplacian)


def line_outage_factors(*, buses: list[Any], branches: list[Branch], isf: np.ndarray) -> np.ndarray:
    incidence = _reduced_incidence_matrix(buses=buses, branches=branches)
    lodf = isf @ incidence.T
    for index in range(lodf.shape[1]):
        denominator = 1.0 - lodf[index, index]
        if abs(denominator) < 1e-12:
            lodf[:, index] = 0.0
        else:
            lodf[:, index] *= 1.0 / denominator
        lodf[index, index] = -1.0
    return lodf


def _reduced_incidence_matrix(*, buses: list[Any], branches: list[Branch]) -> np.ndarray:
    matrix = np.zeros((len(branches), len(buses) - 1))
    for branch in branches:
        if branch.source.offset > 0:
            matrix[branch.offset - 1, branch.source.offset - 1] = 1.0
        if branch.target.offset > 0:
            matrix[branch.offset - 1, branch.target.offset - 1] = -1.0
    return matrix


def _check_shift_factors_model(model: Any) -> None:
    for scenario in model.instance.scenarios:
        for branch in scenario["branches"]:
            if branch.invest > 0.0:
                raise ValueError(
                    "ShiftFactorsTransmissionExt does not support branch investment. "
                    f"Branch '{branch.name}' has investment cost {branch.invest}."
                )
            if np.isfinite(branch.angle_diff_min) or np.isfinite(branch.angle_diff_max):
                raise ValueError(
                    f"Branch '{branch.name}' has finite angle difference limits, "
                    "which are not supported by ShiftFactorsTransmissionExt."
                )
        for contingency in scenario["contingencies"]:
            if len(contingency.branches) != 1:
                raise ValueError(
                    "ShiftFactorsTransmissionExt only supports contingencies with exactly one outage branch. "
                    f"Contingency '{contingency.name}' has {len(contingency.branches)} branches."
                )


def _ensure_shift_factor_data(scenario: UnitCommitmentScenario, isf_cutoff: float, lodf_cutoff: float) -> None:
    if len(scenario["branches"]) == 0:
        return
    if "isf" not in scenario or "lodf" not in scenario:
        compute_shift_factor_data(scenario, isf_cutoff, lodf_cutoff)


def compute_base_flows_from_injections(scenario: UnitCommitmentScenario, net_injections_by_bus: dict[str, list[float]]) -> dict[str, list[float]]:
    _ensure_shift_factor_data(scenario, 0.005, 0.001)
    if len(scenario["branches"]) == 0:
        return {}
    injections = np.array(
        [net_injections_by_bus[bus.name] for bus in scenario["bus"] if bus.offset > 0],
        dtype=float,
    )
    flows = scenario["isf"] @ (injections - _shunt_loss_matrix(scenario, injections.shape[1]))
    return {
        branch.name: [float(value) for value in flows[branch.offset - 1, :]]
        for branch in scenario["branches"]
    }


def compute_contingency_flows_from_base_flows(scenario: UnitCommitmentScenario, base_flows_by_branch: dict[str, list[float]]) -> dict[str, dict[str, list[float]]]:
    _ensure_shift_factor_data(scenario, 0.005, 0.001)
    if len(scenario["branches"]) == 0:
        return {}
    base_flows = np.array(
        [base_flows_by_branch[branch.name] for branch in scenario["branches"]],
        dtype=float,
    )
    flow_by_contingency: dict[str, dict[str, list[float]]] = {}
    for contingency in scenario["contingencies"]:
        if len(contingency.branches) != 1:
            continue
        outage_index = contingency.branches[0].offset - 1
        post_flows = base_flows + scenario["lodf"][:, outage_index : outage_index + 1] @ base_flows[outage_index : outage_index + 1, :]
        flow_by_contingency[contingency.name] = {
            branch.name: [float(value) for value in post_flows[branch.offset - 1, :]]
            for branch in scenario["branches"]
        }
    return flow_by_contingency
def _add_dc_overflow_and_flow_vars(model: Any) -> None:
    for scenario in model.instance.scenarios:
        for branch in scenario["branches"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, branch.name, time)
                if branch.flow_limit_penalty[time - 1] < 0:
                    overflow = model.store("overflow")[key] = 0.0
                else:
                    overflow = model.add_var("overflow", key, lb=0.0)
                    model.add_to_objective(overflow, branch.flow_limit_penalty[time - 1] * scenario["probability"])
                model.add_var("flow", key, lb=-model.infinity, ub=model.infinity)
        for contingency in scenario["contingencies"]:
            for branch in scenario["branches"]:
                for time in range(1, model.instance.time + 1):
                    model.add_var("flow_cont", (scenario.name, contingency.name, branch.name, time), lb=-model.infinity, ub=model.infinity)


def _add_shift_factor_flow_constraints(model: Any) -> None:
    ni = model.store("ni")
    flow = model.store("flow")
    overflow = model.store("overflow")
    flow_cont = model.store("flow_cont")
    for scenario in model.instance.scenarios:
        branches = scenario["branches"]
        if len(branches) == 0:
            continue
        buses = scenario["bus"]
        isf = scenario["isf"]
        lodf = scenario["lodf"]
        shunt_loss = _shunt_loss_matrix(scenario, model.instance.time)
        branch_shunt_corr = isf @ shunt_loss
        for branch in branches:
            branch_index = branch.offset - 1
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, branch.name, time)
                expr = 0.0
                for bus in buses:
                    if bus.offset == 0:
                        continue
                    coef = isf[branch_index, bus.offset - 1]
                    if coef != 0.0:
                        expr += coef * ni[(scenario.name, bus.name, time)]
                model.add_constr("eq_flow_def", key, flow[key] == expr - branch_shunt_corr[branch_index, time - 1])
                model.add_constr("eq_flow_limit_ub", key, flow[key] <= branch.normal_flow_limit[time - 1] + overflow[key])
                model.add_constr("eq_flow_limit_lb", key, flow[key] >= -branch.normal_flow_limit[time - 1] - overflow[key])
        for contingency in scenario["contingencies"]:
            for outage_branch in contingency.branches:
                outage_index = outage_branch.offset - 1
                for monitored_branch in branches:
                    monitored_index = monitored_branch.offset - 1
                    lodf_coef = lodf[monitored_index, outage_index]
                    for time in range(1, model.instance.time + 1):
                        key = (scenario.name, contingency.name, monitored_branch.name, time)
                        base_key = (scenario.name, monitored_branch.name, time)
                        expr = 0.0
                        for bus in buses:
                            if bus.offset == 0:
                                continue
                            total_coef = isf[monitored_index, bus.offset - 1]
                            if lodf_coef != 0.0:
                                outage_coef = isf[outage_index, bus.offset - 1]
                                if outage_coef != 0.0:
                                    total_coef += lodf_coef * outage_coef
                            if total_coef != 0.0:
                                expr += total_coef * ni[(scenario.name, bus.name, time)]
                        shunt_corr = branch_shunt_corr[monitored_index, time - 1] + lodf_coef * branch_shunt_corr[outage_index, time - 1]
                        model.add_constr("eq_flow_cont_def", key, flow_cont[key] == expr - shunt_corr)
                        model.add_constr("eq_flow_cont_limit_ub", key, flow_cont[key] <= monitored_branch.emergency_flow_limit[time - 1] + overflow[base_key])
                        model.add_constr("eq_flow_cont_limit_lb", key, flow_cont[key] >= -monitored_branch.emergency_flow_limit[time - 1] - overflow[base_key])


def _shunt_loss_matrix(scenario: UnitCommitmentScenario, time_horizon: int) -> np.ndarray:
    shunt_loss = np.zeros((len(scenario["bus"]) - 1, time_horizon))
    for bus in scenario["bus"]:
        if bus.offset == 0:
            continue
        for time in range(1, time_horizon + 1):
            shunt_loss[bus.offset - 1, time - 1] = sum(
                shunt.conductance * scenario["base_mva"]
                for shunt in scenario["shunts_by_bus"].get(bus.name, [])
                if shunt.status[time - 1]
            )
    return shunt_loss


def _add_simplified_dc_flow_constraints(model: Any) -> None:
    ni = model.store("ni")
    flow = model.store("flow")
    overflow = model.store("overflow")
    flow_cont = model.store("flow_cont")
    for scenario in model.instance.scenarios:
        for branch in scenario["branches"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, branch.name, time)
                expr = ni[(scenario.name, branch.source.name, time)] - ni[(scenario.name, branch.target.name, time)]
                model.add_constr("eq_flow_def", key, flow[key] == expr)
                model.add_constr("eq_flow_limit_ub", key, flow[key] <= branch.normal_flow_limit[time - 1] + overflow[key])
                model.add_constr("eq_flow_limit_lb", key, flow[key] >= -branch.normal_flow_limit[time - 1] - overflow[key])
        for contingency in scenario["contingencies"]:
            for branch in scenario["branches"]:
                for time in range(1, model.instance.time + 1):
                    key = (scenario.name, contingency.name, branch.name, time)
                    base_key = (scenario.name, branch.name, time)
                    if branch in contingency.branches:
                        model.add_constr("eq_flow_cont_def", key, flow_cont[key] == 0.0)
                    else:
                        model.add_constr("eq_flow_cont_def", key, flow_cont[key] == flow[base_key])
                    model.add_constr("eq_flow_cont_limit_ub", key, flow_cont[key] <= branch.emergency_flow_limit[time - 1] + overflow[base_key])
                    model.add_constr("eq_flow_cont_limit_lb", key, flow_cont[key] >= -branch.emergency_flow_limit[time - 1] - overflow[base_key])
