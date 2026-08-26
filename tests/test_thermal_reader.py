from __future__ import annotations

from pathlib import Path

import unitcommitment as uc


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


def test_read_thermal_units_from_case14():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    scenario = instance.scenarios[0]

    assert len(scenario["thermal"]) == 6
    g1 = scenario["thermal_by_name"]["g1"]
    assert g1.bus.name == "b1"
    assert g1.min_power == [100.0, 100.0, 100.0, 100.0]
    assert g1.max_power == [135.0, 135.0, 135.0, 135.0]
    assert g1.min_power_cost == [1400.0, 1400.0, 1400.0, 1400.0]
    assert [segment.mw for segment in g1.cost_segments] == [
        [10.0, 10.0, 10.0, 10.0],
        [20.0, 20.0, 20.0, 20.0],
        [5.0, 5.0, 5.0, 5.0],
    ]
    assert [segment.cost for segment in g1.cost_segments] == [
        [20.0, 20.0, 20.0, 20.0],
        [30.0, 30.0, 30.0, 30.0],
        [40.0, 40.0, 40.0, 40.0],
    ]


def test_read_startup_categories_and_defaults():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    g2 = instance.scenarios[0]["thermal_by_name"]["g2"]

    assert [(category.delay, category.cost) for category in g2.startup_categories] == [
        (1, 3000.0),
        (4, 4000.0),
    ]
    assert g2.min_uptime == 4
    assert g2.min_downtime == 4
    assert g2.initial_status == -8
    assert g2.initial_power == 0
    assert g2.shutdown_cost == 0.0
    assert g2.commitment_status == [None, None, None, None]


def test_read_reserve_eligibility():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    scenario = instance.scenarios[0]
    reserve = scenario["reserves_by_name"]["r1"]

    assert reserve.type == "spinning"
    assert reserve.amount == [100.0, 100.0, 100.0, 100.0]
    assert reserve.shortfall_penalty == 1000.0
    assert [unit.name for unit in reserve.thermal_units] == ["g2", "g3", "g4", "g5", "g6"]
    assert [reserve.name for reserve in scenario["thermal_by_name"]["g2"].reserves] == ["r1"]
