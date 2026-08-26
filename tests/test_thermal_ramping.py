from __future__ import annotations

from pathlib import Path

import pytest

import unitcommitment as uc

pytest.importorskip("coptpy")


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


def test_build_model_adds_default_thermal_ramping_constraints():
    instance = uc.read(FIXTURES / "thermal_ramp.json", extensions=[uc.ThermalExt(), uc.CopperPlateTransmissionExt()])
    model = uc.build_model(instance)

    assert ("s1", "g1", 1) in model.constraints["eq_ramp_up"]
    assert ("s1", "g1", 2) in model.constraints["eq_ramp_up"]
    assert ("s1", "g1", 1) in model.constraints["eq_ramp_down"]
    assert ("s1", "g1", 2) in model.constraints["eq_ramp_down"]
    assert ("s1", "g1", 1) in model.constraints["eq_slimit_b"]
    assert ("s1", "g1", 1) in model.constraints["eq_slimit_c"]


def test_thermal_ramping_rejects_forced_adjacent_production_jump():
    instance = uc.read(FIXTURES / "thermal_ramp.json", extensions=[uc.ThermalExt(), uc.CopperPlateTransmissionExt()])
    model = uc.build_model(instance)

    model.add_constr("force_prod_t1", "g1", model.variables["prod_above"][("s1", "g1", 1)] == 0.0)
    model.add_constr("force_prod_t2", "g1", model.variables["prod_above"][("s1", "g1", 2)] == 80.0)
    model.auto_store_solution = False
    model.optimize()

    assert model.termination_status == "INFEASIBLE"


def test_thermal_ramping_allows_adjacent_production_within_limit():
    instance = uc.read(FIXTURES / "thermal_ramp.json", extensions=[uc.ThermalExt(), uc.CopperPlateTransmissionExt()])
    model = uc.build_model(instance)

    model.add_constr("force_prod_t1", "g1", model.variables["prod_above"][("s1", "g1", 1)] == 0.0)
    model.add_constr("force_prod_t2", "g1", model.variables["prod_above"][("s1", "g1", 2)] == 30.0)
    model.optimize()
    production = model.solution()["Thermal: Production (MW)"]["g1"]

    assert model.termination_status == "OPTIMAL"
    assert production[1] - production[0] == pytest.approx(30.0)
