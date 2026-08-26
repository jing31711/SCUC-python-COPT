from __future__ import annotations

from pathlib import Path

import pytest

import unitcommitment as uc

pytest.importorskip("coptpy")


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


def test_default_reader_populates_remaining_component_collections():
    instance = uc.read(FIXTURES / "virtual.json")
    scenario = instance.scenarios[0]

    assert "profiled" in scenario
    assert "psload" in scenario
    assert "virtual" in scenario
    assert "storage" in scenario
    assert "branches" in scenario
    assert "interfaces" in scenario
    assert len(scenario["virtual"]) == 4
    assert scenario["virtual_by_name"]["vt_inc1"].type == "inc"


def test_build_model_adds_remaining_component_variables():
    instance = uc.read(FIXTURES / "virtual.json")
    model = uc.build_model(instance)

    assert ("s1", "vt_inc1", 1) in model.variables["vt_cleared"]
    assert "eq_power_balance" in model.constraints
    assert "eq_net_injection" in model.constraints


def test_build_model_with_storage_fixture_adds_storage_variables():
    instance = uc.read(FIXTURES / "case14-storage.json.gz")
    model = uc.build_model(instance)
    storage = instance.scenarios[0]["storage"][0]
    key = ("s1", storage.name, 1)

    assert key in model.variables["storage_level"]
    assert key in model.variables["charge_rate"]
    assert key in model.variables["discharge_rate"]
    assert key in model.constraints["eq_storage_transition"]


def test_storage_transition_scales_power_by_30min_time_step():
    instance = uc.read(
        FIXTURES / "storage_30min.json",
        extensions=[uc.StorageExt(), uc.CopperPlateTransmissionExt()],
    )
    model = uc.build_model(instance)
    key = ("s1", "su1", 1)

    model.add_constr("test_charge", key, model.variables["charge_rate"][key] == 10.0)
    model.add_constr("test_discharge", key, model.variables["discharge_rate"][key] == 0.0)
    model.optimize()

    assert model.termination_status == "OPTIMAL"
    assert model.value(model.variables["storage_level"][key]) == pytest.approx(5.0)


def test_storage_charge_and_discharge_rates_are_nonnegative():
    instance = uc.read(
        FIXTURES / "storage_30min.json",
        extensions=[uc.StorageExt(), uc.CopperPlateTransmissionExt()],
    )
    model = uc.build_model(instance)
    key = ("s1", "su1", 1)

    assert model.variables["charge_rate"][key].lb == 0.0
    assert model.variables["discharge_rate"][key].lb == 0.0


def test_build_model_with_interface_fixture_adds_interface_variables():
    instance = uc.read(FIXTURES / "case14" / "interface.json")
    model = uc.build_model(instance)
    interface = instance.scenarios[0]["interfaces"][0]
    key = ("s1", interface.name, 1)

    assert key in model.variables["interface_flow"]
    assert key in model.constraints["eq_interface_flow_def"]
