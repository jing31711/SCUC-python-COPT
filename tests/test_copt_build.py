from __future__ import annotations

from pathlib import Path

import pytest

import unitcommitment as uc

pytest.importorskip("coptpy")


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


def test_build_model_adds_bus_variables_and_constraints():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)
    key = ("s1", "b1", 1)

    assert key in model.variables["curtail"]
    assert key in model.variables["reactive_curtail"]
    assert key in model.constraints["eq_net_injection"]
    assert key in model.constraints["eq_net_reactive_injection"]


def test_build_model_adds_thermal_variables():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)

    assert ("g1", 1) in model.variables["is_on"]
    assert ("g1", 1) in model.variables["switch_on"]
    assert ("g1", 1) in model.variables["switch_off"]
    assert ("g1", 1, 1) in model.variables["startup"]
    assert ("s1", "g1", 1) in model.variables["prod_above"]
    assert ("s1", "g1", 1, 1) in model.variables["segprod"]
    assert ("s1", "r1", "g2", 1) in model.variables["reserve"]
    assert ("s1", "r1", 1) in model.variables["reserve_shortfall"]


def test_build_model_adds_thermal_output_to_net_injection():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)
    expr = model.variables["net_injection"][("s1", "b1", 1)]

    assert str(model.variables["prod_above"][("s1", "g1", 1)]) in str(expr)
    assert str(model.variables["is_on"][("g1", 1)]) in str(expr)


def test_build_model_collects_objective_terms():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)
    is_on_g1_t1 = model.variables["is_on"][("g1", 1)]
    segprod_g1_t1_k1 = model.variables["segprod"][("s1", "g1", 1, 1)]

    assert any(term is is_on_g1_t1 and coefficient == 1400.0 for term, coefficient in model.objective_terms)
    assert any(term is segprod_g1_t1_k1 and coefficient == 20.0 for term, coefficient in model.objective_terms)
    assert len(model.objective_terms) > 0


def test_build_model_adds_thermal_status_constraints():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)

    assert ("g1", 1) in model.constraints["eq_binary_link"]
    assert ("g1", 1) in model.constraints["eq_min_uptime"]
    assert ("g1", 1) in model.constraints["eq_min_downtime"]
    assert ("g1", 1) in model.constraints["eq_switch_on_off"]
    assert ("g1", 1) in model.constraints["eq_startup_choose"]
    assert ("g1", 1, 1) in model.constraints["eq_startup_restrict"]


def test_build_model_adds_thermal_production_constraints():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)

    assert ("s1", "g1", 1) in model.constraints["eq_prod_limit"]
    assert ("s1", "g1", 1) in model.constraints["eq_prod_above_def"]


def test_build_model_adds_thermal_reserve_constraints():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)

    assert ("s1", "r1", 1) in model.constraints["eq_min_reserve"]
    assert len(model.constraints["eq_min_reserve"]) == instance.time
