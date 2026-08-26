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

model.inner.solve()
solution = uc.store_solution(model)
uc.write("solution.json", solution)
```

## Run tests

```bash
python -m pytest tests -q
```

## Notes

- This is a Python/COPT port, not the original Julia/JuMP package.
- Model variables and constraints are organized through centralized stores in `UnitCommitmentModel`.
- Extensions are implemented as Python objects with methods such as `build_model(model)`.
- Solving requires a valid COPT license; model construction and some tests also require `coptpy`.

## License

See `LICENSE.md`.
