from __future__ import annotations

import json
from pathlib import Path

import pytest

import unitcommitment as uc


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


def test_read_deterministic_fixture():
    instance = uc.read(FIXTURES / "case14" / "base.json")

    assert instance.time == 4
    assert len(instance.scenarios) == 1
    scenario = instance.scenarios[0]
    assert scenario.name == "s1"
    assert scenario["probability"] == 1.0
    assert scenario["power_balance_penalty"] == [100000] * 4
    assert len(scenario["bus"]) == 14
    assert scenario["bus_by_name"]["b1"].name == "b1"
    assert scenario["bus_by_name"]["b1"].load == [0.0, 0.0, 0.0, 0.0]


def test_read_stochastic_fixtures_normalizes_probabilities():
    instance = uc.read([
        FIXTURES / "case14" / "base.json",
        FIXTURES / "case14" / "congested.json",
    ])

    assert instance.time == 4
    assert [scenario.name for scenario in instance.scenarios] == ["base", "congested"]
    assert [scenario["probability"] for scenario in instance.scenarios] == [0.5, 0.5]


def test_read_gz_fixture():
    instance = uc.read(FIXTURES / "case14.json.gz")

    assert instance.time > 0
    assert instance.scenarios[0].name == "s1"
    assert len(instance.scenarios[0]["bus"]) > 0


def test_repair_is_noop_for_clean_instances():
    instance = uc.read(FIXTURES / "case14" / "base.json", repair=True)

    assert instance.time == 4


def test_repair_raises_when_instance_requires_unsupported_fix(tmp_path):
    data = json.loads((FIXTURES / "case14" / "base.json").read_text())
    data["Generators"]["g1"]["Startup limit (MW)"] = 80
    path = tmp_path / "needs_repair.json"
    path.write_text(json.dumps(data))

    with pytest.raises(NotImplementedError, match="Automatic repair is not implemented"):
        uc.read(path)

    instance = uc.read(path, repair=False)
    assert instance.scenarios[0]["thermal_by_name"]["g1"].startup_limit == 80


def test_write_solution_json(tmp_path):
    path = tmp_path / "solution.json"
    uc.write(path, {"Summary": {"Solver: Termination status": "OPTIMAL"}})

    assert json.loads(path.read_text()) == {
        "Summary": {"Solver: Termination status": "OPTIMAL"}
    }
