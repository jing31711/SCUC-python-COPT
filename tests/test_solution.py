from __future__ import annotations

from pathlib import Path

import pytest

import unitcommitment as uc
from unitcommitment.solution import store_solution

pytest.importorskip("coptpy")


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


def test_optimize_stores_solution_for_simple_lmp_fixture():
    instance = uc.read(FIXTURES / "lmp_simple_test_1.json")
    model = uc.build_model(instance)
    model.optimize()
    solution = model.solution()

    assert solution["Summary"]["Solver: Termination status"] == "OPTIMAL"
    assert "Thermal: Production (MW)" in solution
    assert "Bus: Load curtail (MW)" in solution


def test_validate_accepts_stored_solution():
    instance = uc.read(FIXTURES / "lmp_simple_test_1.json")
    model = uc.build_model(instance)
    model.optimize()

    assert uc.validate(instance, model.data["solution"])


def test_default_extensions_do_not_enable_unimplemented_conventional_lmp():
    instance = uc.read(FIXTURES / "lmp_simple_test_1.json")

    assert "lmp" not in instance.extension_by_slot


def test_explicit_conventional_lmp_raises_not_implemented():
    instance = uc.read(FIXTURES / "lmp_simple_test_1.json", extensions=[uc.ConventionalLMP()])

    with pytest.raises(NotImplementedError, match="ConventionalLMP is not implemented"):
        uc.build_model(instance)
def test_build_model_initializes_default_optional_stores():
    instance = uc.read(FIXTURES / "lmp_simple_test_1.json", extensions=[uc.CopperPlateTransmissionExt()])
    model = uc.build_model(instance)

    for store in ["storage_level", "flow", "interface_flow", "reserve", "startup"]:
        assert store in model.variables
        assert isinstance(model.variables[store], dict)
    for store in ["eq_ramp_up", "eq_storage_transition", "eq_interface_flow_def"]:
        assert store in model.constraints
        assert isinstance(model.constraints[store], dict)


def test_store_solution_skips_missing_optional_component_stores():
    instance = uc.read(FIXTURES / "lmp_simple_test_1.json")
    model = uc.build_model(instance)
    model.optimize()
    for store in ["reserve", "segprod", "startup", "storage_level", "flow", "interface_flow"]:
        model.variables.pop(store, None)

    store_solution(model)
    solution = model.solution()

    assert solution["Thermal: Reserve (MW)"] == {}
    assert solution["Thermal: Production cost ($)"] == {}
    assert solution["Thermal: Startup cost ($)"] == {}
    assert solution["Storage: Level (MWh)"] == {}
    assert solution["Branch: Flow (MW)"] == {}
