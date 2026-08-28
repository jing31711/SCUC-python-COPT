#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re
import shutil
from datetime import datetime
from pathlib import Path
from typing import Any

try:
    import unitcommitment
except ModuleNotFoundError as exc:
    if exc.name == "unitcommitment":
        raise SystemExit(
            "The unitcommitment package is not installed. "
            "From the project root, run: python -m pip install -e ."
        ) from exc
    raise


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Build and solve SCUC cases with Python unitcommitment and COPT/coptpy.")
    parser.add_argument("--input", action="append", required=True, help="UnitCommitment JSON or JSON.GZ input file. Pass multiple times for stochastic cases.")
    parser.add_argument("--data-dir", default="data", help="Data directory for run artifacts. Default: data")
    parser.add_argument("--case-name", help="Case label used in the run directory.")
    parser.add_argument("--model-format", choices=["lp", "mps"], default="lp", help="Model export format. Default: lp")
    parser.add_argument("--solve", action="store_true", help="Solve with COPT/coptpy after building the model.")
    parser.add_argument("--time-limit", type=float, help="COPT TimeLimit parameter in seconds.")
    parser.add_argument("--rel-gap", type=float, help="COPT RelGap parameter.")
    return parser.parse_args()


def sanitize_case_name(raw: str) -> str:
    name = re.sub(r"[^a-z0-9]+", "-", raw.lower()).strip("-")
    return name or "scuc"


def set_solver_param(model: Any, name: str, value: Any) -> None:
    try:
        model.inner.setParam(name, value)
    except Exception as exc:
        raise RuntimeError(f"Failed to set COPT parameter {name}={value!r}") from exc


def export_model(model: Any, path: Path) -> None:
    if not hasattr(model.inner, "write"):
        raise RuntimeError("The local coptpy model object does not expose a write() method for model export.")
    model.inner.write(str(path))


def write_summary(path: Path, values: dict[str, Any]) -> None:
    path.write_text("".join(f"{key}={value}\n" for key, value in values.items() if value not in (None, "")), encoding="utf-8")


def has_incumbent(model: Any) -> bool:
    """COPT exposes an objective only when it has a usable primal solution."""
    return model.objective_value is not None


def main() -> int:
    args = parse_args()
    input_paths = [Path(value) for value in args.input]
    if not all(path.exists() for path in input_paths):
        missing = [str(path) for path in input_paths if not path.exists()]
        raise FileNotFoundError(f"Input file(s) not found: {', '.join(missing)}")

    source_label = args.case_name or (input_paths[0].stem if len(input_paths) == 1 else "stochastic")
    case_name = sanitize_case_name(source_label)
    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    run_dir = Path(args.data_dir) / "scuc-runs" / f"{stamp}-{case_name}"
    input_dir = run_dir / "input"
    input_dir.mkdir(parents=True, exist_ok=True)

    for path in input_paths:
        shutil.copy2(path, input_dir / path.name)

    instance_arg: str | list[str]
    if len(input_paths) == 1:
        instance_arg = str(input_paths[0])
    else:
        instance_arg = [str(path) for path in input_paths]

    instance = unitcommitment.read(instance_arg)
    model = unitcommitment.build_model(instance)

    if args.time_limit is not None:
        set_solver_param(model, "TimeLimit", args.time_limit)
    if args.rel_gap is not None:
        set_solver_param(model, "RelGap", args.rel_gap)

    model_path = run_dir / f"model.{args.model_format}"
    export_model(model, model_path)

    solution_path: Path | None = None
    status = "NOT_SOLVED"
    objective = None
    gap = None
    if args.solve:
        # Avoid UnitCommitmentModel's automatic solution extraction for statuses
        # without an incumbent; COPT then has no variable values to retrieve.
        model.auto_store_solution = False
        model.optimize()
        status = model.termination_status
        objective = model.objective_value
        gap = model.relative_gap
        if has_incumbent(model):
            from unitcommitment.solution import store_solution

            store_solution(model)
            solution_path = run_dir / "solution.json"
            unitcommitment.write(solution_path, model.solution())

    summary_path = run_dir / "summary.txt"
    write_summary(
        summary_path,
        {
            "run_dir": run_dir,
            "inputs": json.dumps([str(path) for path in input_paths]),
            "model": model_path,
            "solver": "coptpy/COPT" if args.solve else "not_solved",
            "status": status,
            "objective": objective,
            "relative_gap": gap,
            "solution": solution_path,
        },
    )

    print(f"SCUC run directory: {run_dir}")
    print(f"Model artifact: {model_path}")
    print(f"Solver: {'coptpy/COPT' if args.solve else 'not_solved'}")
    print(f"Status: {status}")
    if solution_path is not None:
        print(f"Solution: {solution_path}")
    print(f"Summary: {summary_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
