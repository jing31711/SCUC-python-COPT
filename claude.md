---
name: scuc-unitcommitment
description: Build, formulate, export, solve, validate, and analyze Security-Constrained Unit Commitment (SCUC) models with the Python unitcommitment package and COPT/coptpy. Use when the user asks to create, establish, formulate, model, build, solve, run, validate, export, or analyze an SCUC, unit commitment, day-ahead commitment, stochastic UC, generator commitment, power-system commitment, or SCUC model file. Prefer the Python/COPT port for SCUC modeling and solving, and generate model artifacts under a data directory.
---

# SCUC UnitCommitment Python/COPT

## Core Rule

For any request that asks to build or solve an SCUC/unit-commitment model, use the Python package `unitcommitment` as the modeling layer. Do not use Julia, `UnitCommitment.jl`, JuMP, or `julia xxx.jl` commands unless the user explicitly asks for the Julia implementation.

After Python dependencies and COPT/coptpy are installed, this port runs independently from Julia and solves through the COPT Python API.

Create a run directory under the data directory before solving:

```text
data/scuc-runs/YYYYMMDD-HHMMSS-<case-name>/
```

Put all generated artifacts there: copied or generated UC JSON input, exported `.lp` or `.mps` model when requested, COPT log when available, Python `unitcommitment` solution JSON when available, and a short run summary.

## Workflow

1. Identify whether the user supplied a UnitCommitment JSON/JSON.GZ file, a list of stochastic scenario files, or describes a new case in natural language.
2. If the case is described in natural language, create a UnitCommitment-compatible JSON input first. Read `references/unitcommitment-usage.md` for the minimum schema and examples.
3. Load the package directly in Python:

   ```python
   import unitcommitment
   ```

4. Read the instance with `unitcommitment.read(...)`:

   ```python
   instance = unitcommitment.read("case.json")
   ```

   For stochastic cases:

   ```python
   instance = unitcommitment.read(["scenario-1.json", "scenario-2.json"])
   ```

5. Build the model with `unitcommitment.build_model(instance)` and solve with `model.optimize()`:

   ```python
   model = unitcommitment.build_model(instance)
   model.optimize()
   solution = model.solution()
   unitcommitment.write("solution.json", solution)
   ```

6. Report exact artifact paths and solver status. If solving fails, keep the generated input, exported model if any, and summary/log files for diagnosis.

## Python API Differences from Julia

- `unitcommitment.build_model(instance, env=None)` accepts only the `instance` and optional COPT `env` arguments.
- Do not call `build_model(instance, extensions=...)`; that is not supported by the Python port.
- Extensions are selected while reading the instance:

  ```python
  instance = unitcommitment.read(
      "case.json",
      extensions=[unitcommitment.CopperPlateTransmissionExt()],
  )
  model = unitcommitment.build_model(instance)
  ```

- Extension model construction is performed internally through each extension object's method:

  ```python
  extension.build_model(model)
  ```

- This differs from the Julia API. When migrating Julia examples, move extension selection to `unitcommitment.read(...)` and keep `build_model(...)` limited to `instance` plus optional `env`.

## Reusable Runner

Use `scripts/run_scuc.py` when it fits the task. It supports custom JSON input, stochastic multi-file input, model export, Python/COPT solving, and export-only mode.

Examples:

```bash
python /Users/shanshu/Downloads/归档/scripts/run_scuc.py \
  --input data/cases/case14.json \
  --data-dir data \
  --solve \
  --model-format lp
```

```bash
python /Users/shanshu/Downloads/归档/scripts/run_scuc.py \
  --input scenario-1.json --input scenario-2.json \
  --data-dir data \
  --solve \
  --time-limit 300 \
  --rel-gap 0.001
```

If the runner does not match the task, write a small Python script that follows the same artifact and solver rules.

## Environment Checks

Before claiming Python/COPT solving is available, run:

```bash
python -c "import unitcommitment; print('unitcommitment OK')"
python -c "import coptpy; print('coptpy OK')"
pytest tests/ -q
```

COPT must be installed and licensed for `coptpy` to create and solve models. If `coptpy` is unavailable, do not switch to another solver silently; ask the user before changing solvers or using export-only mode.

## Test Command

For this Python port, use:

```bash
pytest tests/ -q
```

## References

Read `references/unitcommitment-usage.md` when creating UC JSON, choosing extension settings, exporting models, or deciding which artifacts to produce.
