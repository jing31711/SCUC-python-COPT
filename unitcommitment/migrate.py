from __future__ import annotations

from typing import Any


def migrate(json_data: dict[str, Any]) -> None:
    version = json_data["Parameters"].get("Version")
    if version is None:
        raise ValueError(
            "The provided input file cannot be loaded because it does not specify what version of UnitCommitment.jl it was written for."
        )
    version_tuple = _version_tuple(version)
    if version_tuple < (0, 3):
        _migrate_to_v03(json_data)
    if version_tuple < (0, 4):
        _migrate_to_v04(json_data)
    if version_tuple < (0, 5):
        _migrate_to_v05(json_data)


def _version_tuple(version: str) -> tuple[int, ...]:
    return tuple(int(part) for part in version.split("."))


def _migrate_to_v03(json_data: dict[str, Any]) -> None:
    reserves = json_data.get("Reserves")
    if reserves is not None and reserves.get("Spinning (MW)") is not None:
        amount = reserves["Spinning (MW)"]
        json_data["Reserves"] = {"r1": {"Type": "spinning", "Amount (MW)": amount}}
        for generator in json_data["Generators"].values():
            if generator.get("Provides spinning reserves?") is True:
                generator["Reserve eligibility"] = ["r1"]


def _migrate_to_v04(json_data: dict[str, Any]) -> None:
    generators = json_data.get("Generators")
    if generators is not None:
        for generator in generators.values():
            if generator.get("Type") is None:
                generator["Type"] = "Thermal"


def _migrate_to_v05(json_data: dict[str, Any]) -> None:
    params = json_data["Parameters"]
    if params.get("Base MVA") is None:
        params["Base MVA"] = 100
    if json_data.get("Transmission lines") is not None:
        json_data["Branches"] = json_data["Transmission lines"]
        del json_data["Transmission lines"]
    branches = json_data.get("Branches")
    if branches is not None:
        for branch_name, branch in branches.items():
            if branch.get("Normal flow limit (MW)") is not None:
                branch["Normal flow limit (MVA)"] = branch["Normal flow limit (MW)"]
                branch["Normal flow limit (MW)"] = None
            if branch.get("Emergency flow limit (MW)") is not None:
                branch["Emergency flow limit (MVA)"] = branch["Emergency flow limit (MW)"]
                branch["Emergency flow limit (MW)"] = None
            if branch.get("Susceptance (S)") is not None:
                susceptance = branch["Susceptance (S)"]
                if susceptance == 0.0:
                    raise ValueError(f"Branch {branch_name} has zero susceptance")
                susceptance_pu = susceptance / params["Base MVA"]
                branch["Resistance (p.u.)"] = 0.0
                branch["Reactance (p.u.)"] = 1.0 / susceptance_pu
                del branch["Susceptance (S)"]
    generators = json_data.get("Generators")
    if generators is not None:
        for generator in generators.values():
            if generator.get("Type") is not None:
                generator["Type"] = generator["Type"].lower().capitalize()
    reserves = json_data.get("Reserves")
    if reserves is not None:
        for reserve in reserves.values():
            if reserve.get("Type") is not None:
                reserve["Type"] = reserve["Type"].lower().capitalize()
    contingencies = json_data.get("Contingencies")
    if contingencies is not None:
        for contingency in contingencies.values():
            if contingency.get("Affected lines") is not None:
                contingency["Affected branches"] = contingency["Affected lines"]
                contingency["Affected lines"] = None
