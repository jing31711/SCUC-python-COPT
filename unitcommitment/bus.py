from __future__ import annotations

from .model import UnitCommitmentModel


def build_bus_model(model: UnitCommitmentModel) -> None:
    _add_bus_vars(model)
    _add_bus_objective(model)
    _add_bus_constraints(model)


def _add_bus_vars(model: UnitCommitmentModel) -> None:
    for scenario in model.instance.scenarios:
        for bus in scenario["bus"]:
            for time in range(1, model.instance.time + 1):
                penalty = scenario["power_balance_penalty"][time - 1]
                key = (scenario.name, bus.name, time)
                if penalty < 0:
                    model.store("curtail")[key] = 0.0
                    model.store("reactive_curtail")[key] = 0.0
                else:
                    load = bus.load[time - 1]
                    reactive_load = bus.reactive_load[time - 1]
                    model.add_var(
                        "curtail",
                        key,
                        lb=min(0.0, load),
                        ub=max(0.0, load),
                    )
                    model.add_var(
                        "reactive_curtail",
                        key,
                        lb=min(0.0, reactive_load),
                        ub=max(0.0, reactive_load),
                    )


def _add_bus_objective(model: UnitCommitmentModel) -> None:
    curtail = model.store("curtail")
    reactive_curtail = model.store("reactive_curtail")
    for scenario in model.instance.scenarios:
        for bus in scenario["bus"]:
            for time in range(1, model.instance.time + 1):
                penalty = scenario["power_balance_penalty"][time - 1]
                if penalty < 0:
                    continue
                key = (scenario.name, bus.name, time)
                sign = -1 if bus.load[time - 1] < 0 else 1
                reactive_sign = -1 if bus.reactive_load[time - 1] < 0 else 1
                model.add_to_objective(curtail[key], penalty * scenario["probability"] * sign)
                model.add_to_objective(
                    reactive_curtail[key],
                    penalty * scenario["probability"] * reactive_sign,
                )


def _add_bus_constraints(model: UnitCommitmentModel) -> None:
    ni = model.store("ni")
    qi = model.store("qi")
    net_injection = model.store("net_injection")
    net_reactive_injection = model.store("net_reactive_injection")
    curtail = model.store("curtail")
    reactive_curtail = model.store("reactive_curtail")
    for scenario in model.instance.scenarios:
        for bus in scenario["bus"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, bus.name, time)
                model.add_constr(
                    "eq_net_injection",
                    key,
                    ni[key] == net_injection[key] + curtail[key],
                )
                model.add_constr(
                    "eq_net_reactive_injection",
                    key,
                    qi[key] == net_reactive_injection[key] + reactive_curtail[key],
                )
