from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest

import unitcommitment as uc
from unitcommitment.transmission import (
    compute_base_flows_from_injections,
    compute_contingency_flows_from_base_flows,
)


FIXTURES = Path(__file__).resolve().parents[1] / "test" / "fixtures"


JULIA_CASE14_KEY_ISF = np.array(
    [
        [-0.84, -0.75, -0.67, -0.61, -0.63, -0.66, -0.66, -0.65, -0.65, -0.64, -0.63, -0.63, -0.64],
        [-0.16, -0.25, -0.33, -0.39, -0.37, -0.34, -0.34, -0.35, -0.35, -0.36, -0.37, -0.37, -0.36],
        [0.08, 0.31, 0.50, -0.30, -0.03, 0.36, 0.36, 0.28, 0.23, 0.10, -0.00, 0.02, 0.17],
        [-0.00, -0.00, -0.00, -0.00, -0.00, -0.00, -1.00, -0.00, -0.00, -0.00, -0.00, -0.00, 0.00],
        [-0.00, -0.01, -0.01, 0.01, 0.14, -0.08, -0.08, -0.12, -0.07, 0.03, 0.20, 0.24, -0.40],
    ],
)


JULIA_CASE14_BASE_FLOWS = {
    "l1": [100.0, 94.8, 93.9, 93.7],
    "l2": [31.7, 35.2, 36.1, 36.3],
    "l7": [-41.0, -36.1, -32.6, -33.1],
    "l14": [-92.8, -66.0, -65.1, -60.4],
    "l20": [7.5, 6.3, 4.4, 4.6],
}


JULIA_CASE14_CONTINGENCY_BASE_FLOWS = {
    "l1": [-8.2, -9.2, -9.3, -9.1],
    "l3": [29.8, 13.0, 10.3, 9.3],
    "l7": [-31.3, -26.8, -23.4, -22.4],
    "l14": [-56.7, -33.0, -33.0, -33.0],
    "l20": [12.7, 12.7, 11.1, 10.7],
}


JULIA_CASE14_C1_FLOWS = {
    "l1": [0.0, 0.0, 0.0, 0.0],
    "l3": [31.2, 14.5, 11.9, 10.8],
    "l7": [-27.2, -22.3, -18.8, -17.8],
    "l14": [-56.7, -33.0, -33.0, -33.0],
    "l20": [12.6, 12.6, 11.0, 10.6],
}


def test_shift_factor_reader_stores_case14_isf_lodf_matrices():
    instance = uc.read(FIXTURES / "case14" / "base.json")
    scenario = instance.scenarios[0]

    assert scenario["isf"].shape == (20, 13)
    assert scenario["lodf"].shape == (20, 20)
    np.testing.assert_allclose(scenario["isf"][[0, 1, 6, 13, 19]], JULIA_CASE14_KEY_ISF, atol=0.01)


def test_simplified_transmission_extension_keeps_old_endpoint_injection_model():
    instance = uc.read(FIXTURES / "case14" / "base.json", extensions=[uc.SimplifiedTransmissionExt()])
    scenario = instance.scenarios[0]

    assert "isf" not in scenario
    assert "lodf" not in scenario


@pytest.mark.parametrize(
    ("fixture", "julia_flows"),
    [
        ("base.json", JULIA_CASE14_BASE_FLOWS),
        ("contingency.json", JULIA_CASE14_CONTINGENCY_BASE_FLOWS),
    ],
)
def test_case14_python_isf_reproduces_julia_key_base_flows(fixture: str, julia_flows: dict[str, list[float]]):
    instance = uc.read(FIXTURES / "case14" / fixture, extensions=[uc.ShiftFactorsTransmissionExt(isf_cutoff=0.0, lodf_cutoff=0.0)])
    scenario = instance.scenarios[0]
    injections = _net_injections_for_key_flows(scenario, julia_flows)

    actual_flows = compute_base_flows_from_injections(scenario, injections)

    for branch_name, expected in julia_flows.items():
        np.testing.assert_allclose(actual_flows[branch_name], expected, atol=0.25)


def test_case14_python_lodf_reproduces_julia_key_contingency_flows():
    instance = uc.read(FIXTURES / "case14" / "contingency.json", extensions=[uc.ShiftFactorsTransmissionExt(isf_cutoff=0.0, lodf_cutoff=0.0)])
    scenario = instance.scenarios[0]
    base_flows = _all_branch_base_flows_for_key_flows(scenario, JULIA_CASE14_CONTINGENCY_BASE_FLOWS)

    actual_flows = compute_contingency_flows_from_base_flows(scenario, base_flows)["c1"]

    for branch_name, expected in JULIA_CASE14_C1_FLOWS.items():
        np.testing.assert_allclose(actual_flows[branch_name], expected, atol=0.25)


def _net_injections_for_key_flows(scenario, key_flows: dict[str, list[float]]) -> dict[str, list[float]]:
    branch_names = list(key_flows)
    branch_rows = [scenario["branch_by_name"][branch_name].offset - 1 for branch_name in branch_names]
    target = np.array([key_flows[branch_name] for branch_name in branch_names], dtype=float)
    non_slack_injections = np.linalg.lstsq(scenario["isf"][branch_rows], target, rcond=None)[0]
    injections = {
        bus.name: non_slack_injections[bus.offset - 1].tolist()
        for bus in scenario["bus"]
        if bus.offset > 0
    }
    injections[scenario["bus"][0].name] = (-non_slack_injections.sum(axis=0)).tolist()
    return injections


def _all_branch_base_flows_for_key_flows(scenario, key_flows: dict[str, list[float]]) -> dict[str, list[float]]:
    injections = _net_injections_for_key_flows(scenario, key_flows)
    return compute_base_flows_from_injections(scenario, injections)
