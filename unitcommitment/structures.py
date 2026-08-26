from __future__ import annotations

from dataclasses import dataclass, field
from typing import Any


@dataclass
class Bus:
    name: str
    offset: int
    load: list[float]
    reactive_load: list[float]
    vmin: float
    vmax: float
    bus_type: str


@dataclass
class CostSegment:
    mw: list[float]
    cost: list[float]


@dataclass
class StartupCategory:
    delay: int
    cost: float


@dataclass
class Reserve:
    name: str
    type: str
    amount: list[float]
    thermal_units: list[Any]
    shortfall_penalty: float
    parent: Any = None
    descendants: list[Any] = field(default_factory=list)


@dataclass
class ThermalUnit:
    name: str
    bus: Bus
    max_power: list[float]
    min_power: list[float]
    must_run: list[bool]
    min_power_cost: list[float]
    cost_segments: list[CostSegment]
    min_uptime: int
    min_downtime: int
    ramp_up_limit: float
    ramp_down_limit: float
    startup_limit: float
    shutdown_limit: float
    initial_status: int | None
    initial_power: float | None
    startup_categories: list[StartupCategory]
    shutdown_cost: float
    reserves: list[Reserve]
    non_spinning_capacity: float
    commitment_status: list[bool | None]
    invest: float
    qmin: float = 0.0
    qmax: float = 0.0


@dataclass
class ProfiledUnit:
    name: str
    bus: Bus
    min_power: list[float]
    max_power: list[float]
    cost: list[float]
    invest: float
    qmin: float = 0.0
    qmax: float = 0.0


@dataclass
class PriceSensitiveLoad:
    name: str
    bus: Bus
    demand: list[float]
    revenue: list[float]


@dataclass
class VirtualTransaction:
    name: str
    type: str
    bus_source: Bus
    bus_sink: Bus
    price: list[float]
    max_quantity: list[float]


@dataclass
class StorageUnit:
    name: str
    bus: Bus
    min_level: list[float]
    max_level: list[float]
    simultaneous_charge_and_discharge: list[bool]
    charge_cost: list[float]
    discharge_cost: list[float]
    charge_efficiency: list[float]
    discharge_efficiency: list[float]
    loss_factor: list[float]
    min_charge_rate: list[float]
    max_charge_rate: list[float]
    min_discharge_rate: list[float]
    max_discharge_rate: list[float]
    initial_level: float
    min_ending_level: float
    max_ending_level: float
    invest: float
    qmin: float = 0.0
    qmax: float = 0.0
    apparent_power_limit: float = float("inf")


@dataclass
class Branch:
    name: str
    offset: int
    source: Bus
    target: Bus
    resistance: float
    reactance: float
    susceptance: float
    shunt_conductance: float
    shunt_susceptance: float
    tap_ratio: float
    phase_shift: float
    normal_flow_limit: list[float]
    emergency_flow_limit: list[float]
    flow_limit_penalty: list[float]
    angle_diff_min: float = float("-inf")
    angle_diff_max: float = float("inf")
    invest: float = 0.0
    max_copy: int = 1


@dataclass
class Contingency:
    name: str
    branches: list[Branch]


@dataclass
class ShuntDevice:
    name: str
    bus: Bus
    conductance: float
    susceptance: float
    status: list[bool]


@dataclass
class Interface:
    name: str
    offset: int
    branches: list[Branch]
    weight_by_branch: dict[str, float]
    net_flow_ub: list[float]
    net_flow_lb: list[float]
    flow_limit_penalty: list[float]


@dataclass
class UnitCommitmentScenario:
    name: str
    data: dict[str, Any] = field(default_factory=dict)

    def __getitem__(self, key: str) -> Any:
        return self.data[key]

    def __setitem__(self, key: str, value: Any) -> None:
        self.data[key] = value

    def get(self, key: str, default: Any = None) -> Any:
        return self.data.get(key, default)

    def __contains__(self, key: str) -> bool:
        return key in self.data


@dataclass
class UnitCommitmentInstance:
    time: int
    scenarios: list[UnitCommitmentScenario]
    extensions: list[Any] = field(default_factory=list)
    extension_by_slot: dict[str, Any] = field(default_factory=dict)


class Extension:
    slot: str | None = None

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        return None

    def build_model(self, model: Any) -> None:
        return None

    def store_solution(self, sol: dict[str, Any], model: Any) -> None:
        return None

    def validate(self, instance: UnitCommitmentInstance, solution: dict[str, Any], tol: float = 0.01) -> int:
        return 0


class ThermalExt(Extension):
    slot = "thermal"

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .thermal import read_thermal_json

        read_thermal_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .thermal import build_thermal_model

        build_thermal_model(model)


class ProfiledUnitsExt(Extension):
    slot = "profiled"

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .components import read_profiled_json

        read_profiled_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .components import build_profiled_model

        build_profiled_model(model)


class PriceSensitiveLoadsExt(Extension):
    slot = "psload"

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .components import read_psload_json

        read_psload_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .components import build_psload_model

        build_psload_model(model)


class VirtualTransactionsExt(Extension):
    slot = "virtual"

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .components import read_virtual_json

        read_virtual_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .components import build_virtual_model

        build_virtual_model(model)


class StorageExt(Extension):
    slot = "storage"

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .components import read_storage_json

        read_storage_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .components import build_storage_model

        build_storage_model(model)


class ShiftFactorsTransmissionExt(Extension):
    slot = "transmission"

    def __init__(self, isf_cutoff: float = 0.005, lodf_cutoff: float = 0.001, lazy: bool = False) -> None:
        self.isf_cutoff = isf_cutoff
        self.lodf_cutoff = lodf_cutoff
        self.lazy = lazy

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .transmission import compute_shift_factor_data, read_transmission_json

        read_transmission_json(json_data, scenario)
        compute_shift_factor_data(scenario, self.isf_cutoff, self.lodf_cutoff)

    def build_model(self, model: Any) -> None:
        from .transmission import build_shift_factors_model

        build_shift_factors_model(model, self)


class SimplifiedTransmissionExt(Extension):
    slot = "transmission"

    def __init__(self, lazy: bool = False) -> None:
        self.lazy = lazy

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .transmission import read_transmission_json

        read_transmission_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .transmission import build_simplified_transmission_model

        build_simplified_transmission_model(model, self)


class CopperPlateTransmissionExt(Extension):
    slot = "transmission"

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .transmission import read_transmission_json

        read_transmission_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .transmission import build_copperplate_model

        build_copperplate_model(model)


class InterfaceLimitsExt(Extension):
    slot = "interface"

    def read_json(self, json_data: dict[str, Any], scenario: UnitCommitmentScenario) -> None:
        from .components import read_interface_json

        read_interface_json(json_data, scenario)

    def build_model(self, model: Any) -> None:
        from .components import build_interface_model

        build_interface_model(model)


class ConventionalLMP(Extension):
    slot = "lmp"

    def build_model(self, model: Any) -> None:
        raise NotImplementedError("ConventionalLMP is not implemented in the Python/COPT port yet.")


DEFAULT_EXTENSIONS: list[Extension] = [
    ThermalExt(),
    ProfiledUnitsExt(),
    PriceSensitiveLoadsExt(),
    VirtualTransactionsExt(),
    StorageExt(),
    ShiftFactorsTransmissionExt(),
    InterfaceLimitsExt(),
]
