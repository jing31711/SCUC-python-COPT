---
name: scuc-unitcommitment
description: Build, formulate, export, and solve Security-Constrained Unit Commitment (SCUC) models with UnitCommitment.jl. Use when the user asks to create, establish, formulate, model, build, solve, run, validate, export, or analyze an SCUC, unit commitment, day-ahead commitment, stochastic UC, benchmark UC case, generator commitment, power-system commitment, or SCUC model file. Prefer UnitCommitment.jl for SCUC modeling and solving, and generate model artifacts under a data directory.
---

# SCUC UnitCommitment

## Core Rule

For any request that asks to build or solve an SCUC/unit-commitment model, use `UnitCommitment.jl` as the modeling layer. Do not hand-roll the SCUC MILP unless the user explicitly asks for a from-scratch formulation.

Create a run directory under the data directory before solving:

```text
data/scuc-runs/YYYYMMDD-HHMMSS-<case-name>/
```

Put all generated artifacts there: copied or generated UC JSON input, exported `.lp` or `.mps` model, solver log, raw COPT solution when applicable, UnitCommitment solution JSON when available, and a short run summary.

## Workflow

1. Identify whether the user supplied a UnitCommitment JSON file, asks for a benchmark case, or describes a new case in natural language.
2. If the case is described in natural language, create a UnitCommitment-compatible JSON input first. Read `references/unitcommitment-usage.md` for the minimum schema and examples.
3. Build the model with `UnitCommitment.read` or `UnitCommitment.read_benchmark`, then `UnitCommitment.build_model`.
4. Always export the JuMP model with `JuMP.write_to_file(model.inner, "model.lp")` or `"model.mps"` into the run directory.
5. Prefer COPT for solving:
   - Use `COPT.Optimizer` through `COPT.jl` when `using COPT` succeeds.
   - If `COPT.jl` fails but `copt_cmd` exists, solve the exported LP/MPS with `copt_cmd` and save the solver log and raw solution. State that UnitCommitment `solution.json` was not available from CLI-only solving.
   - Do not silently switch to HiGHS, Cbc, or GLPK unless the user approves or explicitly requests a non-COPT solver.
6. When solved through a JuMP optimizer, call `UnitCommitment.solution(model)` and `UnitCommitment.write("solution.json", solution)`.
7. Report exact artifact paths and solver status. If solving fails, keep the exported model and log for diagnosis.

## Reusable Runner

Use `scripts/run_scuc.jl` when it fits the task. It supports custom JSON input, benchmark names, model export, COPT.jl solving, COPT CLI solving, and export-only mode.

Examples:

```bash
julia /Users/shanshu/.codex/skills/scuc-unitcommitment/scripts/run_scuc.jl \
  --input data/cases/case14.json \
  --data-dir data \
  --solver auto \
  --model-format lp
```

```bash
julia /Users/shanshu/.codex/skills/scuc-unitcommitment/scripts/run_scuc.jl \
  --benchmark matpower/case14/2017-01-01 \
  --data-dir data \
  --solver copt-cmd \
  --time-limit 300
```

If the runner does not match the task, write a small Julia script that follows the same artifact and solver rules.

## Environment Checks

Before claiming COPT solving is available, run:

```bash
julia --version
julia -e 'using UnitCommitment; println("UnitCommitment OK")'
julia -e 'try; using COPT; println("COPT.jl OK"); catch e; println(e); exit(1); end'
command -v copt_cmd
copt_cmd -c "quit"
```

Known local issue: COPT.jl may fail to precompile against a `copt_cmd` build whose version banner is `Cardinal Optimizer SCUC v8.0.2`. In that case, use the exported-model plus `copt_cmd` path and say that the Julia wrapper is unavailable.

## References

Read `references/unitcommitment-usage.md` when creating UC JSON, choosing `read` versus `read_benchmark`, exporting models, or deciding which artifacts to produce.
