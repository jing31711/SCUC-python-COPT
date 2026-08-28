# unitcommitment

Python/COPT port of UnitCommitment.jl for security-constrained unit commitment (SCUC) modeling.

This package provides a command-style Python implementation of the core UnitCommitment.jl data reading, model building, validation, and solution extraction workflow. It uses COPT through `coptpy` as the mathematical optimization backend.

## Requirements

- Python 3.10+
- `numpy`
- `scipy`
- `coptpy`
- A working COPT/Cardinal Optimizer installation and license

## Installation

```bash
python -m pip install -e .
```

For tests:

```bash
python -m pip install -e '.[test]'
```

## Quick start

```python
import unitcommitment as uc

instance = uc.read("test/fixtures/case14/base.json")
model = uc.build_model(instance)

model.optimize()
solution = model.solution()
uc.write("solution.json", solution)
```

## Run tests

```bash
python -m pytest tests -q
```

## Run SCUC cases

Install the package before running the scripts. Running `python scripts/run_scuc.py`
does not add the project root to Python's import path.

```bash
python -m pip install -e .
python scripts/run_scuc.py --input case57/2017-03-01.json.gz --solve --time-limit 300
```

For a long solve, use the background launcher. It redirects standard input and
both output streams, so the process is detached from the terminal on macOS and
Linux without requiring `setsid`.

```bash
scripts/run_scuc_background.sh --input case57/2017-03-01.json.gz --solve --time-limit 300
scripts/scuc_status.sh PID data/scuc-runs/_logs/scuc-YYYYMMDD-HHMMSS.log
```

The COPT solver log is the portable source for row, column, and binary counts;
the scripts deliberately do not query non-portable `coptpy` model statistics.
Use MPS when an LP export is unnecessarily large:

```bash
python scripts/run_scuc.py --input case57/2017-03-01.json.gz --model-format mps
```

`solution.json` stores thermal series as objects keyed by unit name, rather
than a list of units. Inspect it with:

```bash
python scripts/inspect_solution.py data/scuc-runs/RUN/solution.json --unit g1 --periods 36
```

## Notes

- This is a Python/COPT port, not the original Julia/JuMP package.
- Model variables and constraints are organized through centralized stores in `UnitCommitmentModel`.
- Extensions are implemented as Python objects with methods such as `build_model(model)`.
- Solving requires a valid COPT license; model construction and some tests also require `coptpy`.

## License

See `LICENSE.md`.
