from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any

from .structures import UnitCommitmentInstance

try:
    import coptpy as cp
    from coptpy import COPT
except Exception:  # pragma: no cover
    cp = None
    COPT = None


_STATUS_NAMES = {
    "OPTIMAL": "OPTIMAL",
    "INFEASIBLE": "INFEASIBLE",
    "INF_OR_UNB": "INF_OR_UNB",
    "UNBOUNDED": "UNBOUNDED",
    "TIMEOUT": "TIME_LIMIT",
    "NODELIMIT": "NODE_LIMIT",
    "ITERLIMIT": "ITERATION_LIMIT",
    "INTERRUPTED": "INTERRUPTED",
    "NUMERICAL": "NUMERICAL",
}


@dataclass
class UnitCommitmentModel:
    instance: UnitCommitmentInstance
    env: Any = None
    inner: Any = None
    data: dict[str, Any] = field(default_factory=dict)
    variables: dict[str, dict[Any, Any]] = field(default_factory=dict)
    constraints: dict[str, dict[Any, Any]] = field(default_factory=dict)
    objective: Any = None
    objective_terms: list[tuple[Any, float]] = field(default_factory=list)
    binary_variables: list[Any] = field(default_factory=list)
    integer_variables: list[Any] = field(default_factory=list)
    auto_store_solution: bool = False

    def __post_init__(self) -> None:
        if self.inner is not None:
            return
        if cp is None:
            raise RuntimeError("coptpy is required to build a COPT model.")
        if self.env is None:
            self.env = cp.Envr()
        self.inner = self.env.createModel("unit_commitment")
        self.objective = 0.0

    @property
    def infinity(self) -> float:
        return _infinity()

    def store(self, key: str) -> dict[Any, Any]:
        return self.variables.setdefault(key, {})

    def constraint_store(self, key: str) -> dict[Any, Any]:
        return self.constraints.setdefault(key, {})

    def add_var(
        self,
        store: str,
        index: Any,
        *,
        lb: float = 0.0,
        ub: float | None = None,
        vtype: Any = None,
        name: str | None = None,
    ) -> Any:
        if COPT is None:
            raise RuntimeError("coptpy is required to add variables.")
        resolved_vtype = COPT.CONTINUOUS if vtype is None else vtype
        resolved_ub = COPT.INFINITY if ub is None else ub
        variable = self.inner.addVar(
            lb=lb,
            ub=resolved_ub,
            vtype=resolved_vtype,
            name=name or _format_name(store, index),
        )
        self.variables.setdefault(store, {})[index] = variable
        if resolved_vtype == COPT.BINARY:
            self.binary_variables.append(variable)
        elif resolved_vtype == COPT.INTEGER:
            self.integer_variables.append(variable)
        return variable

    def add_binary_var(self, store: str, index: Any, name: str | None = None) -> Any:
        if COPT is None:
            raise RuntimeError("coptpy is required to add variables.")
        return self.add_var(store, index, lb=0.0, ub=1.0, vtype=COPT.BINARY, name=name)

    def add_constr(self, store: str, index: Any, constr_expr: Any, name: str | None = None) -> Any:
        constraint = self.inner.addConstr(constr_expr, name=name or _format_name(store, index))
        self.constraints.setdefault(store, {})[index] = constraint
        return constraint

    def add_to_objective(self, term: Any, coefficient: float = 1.0) -> None:
        self.objective += coefficient * term
        self.objective_terms.append((term, coefficient))

    def set_objective_minimize(self) -> None:
        if COPT is None:
            raise RuntimeError("coptpy is required to set objectives.")
        self.inner.setObjective(self.objective, COPT.MINIMIZE)

    def optimize(self) -> None:
        self.inner.solve()
        if self.auto_store_solution:
            from .solution import store_solution

            store_solution(self)

    @property
    def termination_status(self) -> str:
        status = getattr(self.inner, "status", None)
        if status is None:
            return "UNKNOWN"
        if COPT is None:
            return str(status)
        for constant_name, status_name in _STATUS_NAMES.items():
            if getattr(COPT, constant_name, object()) == status:
                return status_name
        return str(status)

    @property
    def objective_value(self) -> float | None:
        return _get_attr(self.inner, "ObjVal")

    @property
    def objective_bound(self) -> float | None:
        return _get_attr(self.inner, "BestBnd")

    @property
    def solve_time(self) -> float | None:
        return _get_attr(self.inner, "SolvingTime")

    @property
    def relative_gap(self) -> float | None:
        best_obj = _get_attr(self.inner, "BestObj")
        best_bound = _get_attr(self.inner, "BestBnd")
        if best_obj in (None, 0):
            return None
        if best_bound is None:
            return None
        return abs(best_obj - best_bound) / abs(best_obj)

    def value(self, variable_or_constant: Any) -> float:
        if isinstance(variable_or_constant, (int, float)):
            return float(variable_or_constant)
        return float(variable_or_constant.x)

    def solution(self) -> dict[str, Any]:
        if "solution" not in self.data:
            raise RuntimeError("No solution available. Call optimize() first.")
        sol = self.data["solution"]
        if len(sol) == 1:
            return next(iter(sol.values()))
        return sol


def build_model(instance: UnitCommitmentInstance, env: Any = None) -> UnitCommitmentModel:
    from .bus import build_bus_model

    model = UnitCommitmentModel(instance=instance, env=env)
    _init_core_stores(model)
    for extension in instance.extensions:
        extension.build_model(model)
    build_bus_model(model)
    model.set_objective_minimize()
    model.auto_store_solution = True
    return model


def _init_core_stores(model: UnitCommitmentModel) -> None:
    _init_default_stores(model)
    for scenario in model.instance.scenarios:
        _init_default_scenario_collections(scenario)
        for bus in scenario["bus"]:
            for time in range(1, model.instance.time + 1):
                key = (scenario.name, bus.name, time)
                model.add_var("ni", key, lb=-_infinity(), name=_format_name("ni", key))
                model.add_var("qi", key, lb=-_infinity(), name=_format_name("qi", key))
                model.store("net_injection")[key] = -bus.load[time - 1]
                model.store("net_reactive_injection")[key] = -bus.reactive_load[time - 1]


def _init_default_stores(model: UnitCommitmentModel) -> None:
    variable_stores = [
        "charge_rate",
        "curtail",
        "discharge_rate",
        "flow",
        "flow_cont",
        "interface_flow",
        "interface_overflow",
        "invest",
        "invest_storage",
        "is_charging",
        "is_discharging",
        "is_on",
        "loads",
        "net_injection",
        "net_reactive_injection",
        "ni",
        "overflow",
        "prod",
        "prod_above",
        "qg_profiled",
        "qi",
        "qs",
        "reactive_curtail",
        "reserve",
        "reserve_shortfall",
        "segprod",
        "startup",
        "storage_level",
        "switch_off",
        "switch_on",
        "vt_cleared",
    ]
    constraint_stores = [
        "eq_binary_link",
        "eq_commitment_status",
        "eq_ending_level",
        "eq_ending_level_ub",
        "eq_flow_cont_def",
        "eq_flow_cont_limit_lb",
        "eq_flow_cont_limit_ub",
        "eq_flow_def",
        "eq_flow_limit_lb",
        "eq_flow_limit_ub",
        "eq_interface_flow_def",
        "eq_interface_flow_lb",
        "eq_interface_flow_ub",
        "eq_invest_link",
        "eq_invest_prod_lb",
        "eq_invest_prod_ub",
        "eq_invest_storage_level_lb",
        "eq_invest_storage_level_ub",
        "eq_max_charge_rate",
        "eq_max_discharge_rate",
        "eq_min_charge_rate",
        "eq_min_discharge_rate",
        "eq_min_downtime",
        "eq_min_reserve",
        "eq_min_uptime",
        "eq_must_run",
        "eq_net_injection",
        "eq_net_reactive_injection",
        "eq_ns_reserve_capacity",
        "eq_power_balance",
        "eq_prod_above_def",
        "eq_prod_limit",
        "eq_ramp_down",
        "eq_ramp_up",
        "eq_simultaneous_charge_and_discharge",
        "eq_slimit_a",
        "eq_slimit_b",
        "eq_slimit_c",
        "eq_slimit_init",
        "eq_startup_choose",
        "eq_startup_restrict",
        "eq_storage_transition",
        "eq_switch_on_off",
    ]
    for store in variable_stores:
        model.store(store)
    for store in constraint_stores:
        model.constraint_store(store)


def _init_default_scenario_collections(scenario: Any) -> None:
    collections = [
        ("branches", []),
        ("contingencies", []),
        ("interfaces", []),
        ("profiled", []),
        ("psload", []),
        ("reserves", []),
        ("shunts", []),
        ("storage", []),
        ("thermal", []),
        ("virtual", []),
    ]
    mappings = [
        ("branch_by_name", {}),
        ("contingencies_by_name", {}),
        ("interface_by_name", {}),
        ("profiled_by_name", {}),
        ("psload_by_name", {}),
        ("reserves_by_name", {}),
        ("shunt_by_name", {}),
        ("shunts_by_bus", {}),
        ("storage_by_name", {}),
        ("thermal_by_name", {}),
        ("virtual_by_name", {}),
    ]
    for key, value in collections + mappings:
        scenario.data.setdefault(key, value)


def _format_name(store: str, index: Any) -> str:
    if isinstance(index, tuple):
        joined = ",".join(str(part) for part in index)
    else:
        joined = str(index)
    return f"{store}[{joined}]"


def _infinity() -> float:
    if COPT is not None:
        return COPT.INFINITY
    return 1e30


def _get_attr(model: Any, attr: str) -> Any:
    try:
        return model.getAttr(attr)
    except Exception:
        return None
