from __future__ import annotations

from pathlib import Path

import pytest

import unitcommitment as uc

coptpy = pytest.importorskip("coptpy")


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


def test_build_model_initializes_net_injection_stores():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.build_model(instance)

    key = ("s1", "b1", 1)
    assert key in model.variables["ni"]
    assert key in model.variables["qi"]
    assert str(model.variables["prod_above"][("s1", "g1", 1)]) in str(model.variables["net_injection"][key])
    assert model.variables["net_reactive_injection"][key] == -instance.scenarios[0]["bus_by_name"]["b1"].reactive_load[0]


def test_adapter_adds_variables_constraints_and_solves_small_lp():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.UnitCommitmentModel(instance)
    x = model.add_var("x", "only", lb=0.0, ub=10.0)
    model.add_constr("c", "min", x >= 1.0)
    model.add_to_objective(x, 2.0)
    model.set_objective_minimize()

    model.optimize()

    assert model.termination_status == "OPTIMAL"
    assert model.value(x) == pytest.approx(1.0)
    assert model.objective_value == pytest.approx(2.0)
    assert model.solve_time is not None


def test_adapter_tracks_binary_variables():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    model = uc.UnitCommitmentModel(instance)
    y = model.add_binary_var("is_on", ("g1", 1))

    assert y in model.binary_variables
    assert model.variables["is_on"][("g1", 1)] is y
