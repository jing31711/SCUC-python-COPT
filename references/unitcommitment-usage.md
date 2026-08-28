# Python unitcommitment Usage Notes

Use these notes as a compact guide for the Python/COPT port of UnitCommitment-style SCUC modeling.

## Basic Flow

```python
import unitcommitment

instance = unitcommitment.read("case.json")
model = unitcommitment.build_model(instance)
model.optimize()
solution = model.solution()
unitcommitment.write("data/scuc-runs/run/solution.json", solution)
```

For stochastic cases, pass multiple scenario files to `read`:

```python
instance = unitcommitment.read(["scenario-1.json", "scenario-2.json"])
```

The Python port builds and solves through COPT/coptpy. After dependencies and COPT are installed, Julia is not required.

## Model Construction API

`build_model` has this Python signature:

```python
model = unitcommitment.build_model(instance)
```

or, when reusing a COPT environment:

```python
import coptpy as cp
import unitcommitment

env = cp.Envr()
instance = unitcommitment.read("case.json")
model = unitcommitment.build_model(instance, env=env)
```

Do not pass `extensions=` to `build_model`; the Python API does not support it.

Select extensions when reading the instance:

```python
instance = unitcommitment.read(
    "case.json",
    extensions=[unitcommitment.CopperPlateTransmissionExt()],
)
model = unitcommitment.build_model(instance)
```

Internally, model construction calls extension object methods:

```python
extension.build_model(model)
```

This is the important migration difference from Julia examples.

## Common Extensions

The default extension set includes thermal units, profiled units, price-sensitive loads, virtual transactions, storage, shift-factor transmission, and interface limits.

Use explicit extensions when a task requires a different transmission representation or intentionally limited component set:

```python
instance = unitcommitment.read(
    "case.json",
    extensions=[unitcommitment.SimplifiedTransmissionExt()],
)
```

```python
instance = unitcommitment.read(
    "case.json",
    extensions=[unitcommitment.CopperPlateTransmissionExt()],
)
```

`unitcommitment.ConventionalLMP()` is exposed but not implemented in the Python/COPT port yet. Do not enable it unless the task is specifically to verify that it raises `NotImplementedError`.

## Minimal Deterministic JSON

```json
{
  "Parameters": {
    "Version": "0.5",
    "Time horizon (h)": 4
  },
  "Buses": {
    "b1": {
      "Load (MW)": [100, 150, 200, 250]
    }
  },
  "Generators": {
    "g1": {
      "Bus": "b1",
      "Type": "Thermal",
      "Production cost curve (MW)": [0, 200],
      "Production cost curve ($)": [0, 1000],
      "Initial status (h)": -24,
      "Initial power (MW)": 0
    }
  }
}
```

Use the UnitCommitment input format for richer cases with transmission, reserves, storage, profiled units, price-sensitive loads, virtual transactions, contingencies, or stochastic scenarios.

## Artifact Rules

Use this structure unless the user specifies another data directory:

```text
data/scuc-runs/YYYYMMDD-HHMMSS-case-name/
  input/
  model.lp
  model.mps
  copt.log
  solution.json
  summary.txt
```

Produce a `solution.json` whenever `model.optimize()` succeeds and `model.solution()` is available. Produce `.lp` or `.mps` model files when the user asks for model export or when debugging solver/model issues.

## Solver Notes

Prefer COPT through Python `coptpy`:

```python
import coptpy as cp
import unitcommitment

env = cp.Envr()
instance = unitcommitment.read("case.json")
model = unitcommitment.build_model(instance, env=env)
model.optimize()
```

Solver parameter support depends on the exposed COPT/coptpy API in the local environment. Keep solver parameters explicit and report any unsupported parameter-setting behavior instead of silently ignoring it.

If `coptpy` is unavailable, do not silently switch to HiGHS, Cbc, GLPK, JuMP, or Julia. Ask the user whether to install dependencies, run export-only diagnostics, or use another solver.

## Test Command

Run the Python test suite with:

```bash
pytest tests/ -q
```

Start with the smallest relevant test file when debugging, then run `pytest tests/ -q` before reporting completion.

## Success Criteria

A completed SCUC task should report:

- Input source: generated JSON, supplied JSON, or stochastic scenario list.
- Python package path/version information when relevant.
- Model artifact path when exported.
- Solver path: Python `coptpy`/COPT, export-only, or an explicitly approved fallback.
- Solution artifact path when available.
- Objective/status/gap when the solver reports them.
