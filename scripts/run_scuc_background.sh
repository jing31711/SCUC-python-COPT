#!/usr/bin/env bash
# Start an SCUC run independently of the caller's terminal.
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
runner="$script_dir/run_scuc.py"
launch_dir="$(pwd)"
python_bin="${PYTHON_BIN:-python}"
data_dir="data"
expect_data_dir=false

for arg in "$@"; do
    if "$expect_data_dir"; then
        data_dir="$arg"
        expect_data_dir=false
        continue
    fi
    case "$arg" in
        --data-dir)
            expect_data_dir=true
            ;;
        --data-dir=*)
            data_dir="${arg#--data-dir=}"
            ;;
    esac
done

if "$expect_data_dir"; then
    echo "--data-dir requires a value" >&2
    exit 2
fi

case "$data_dir" in
    /*) log_dir="$data_dir/scuc-runs/_logs" ;;
    *) log_dir="$launch_dir/$data_dir/scuc-runs/_logs" ;;
esac

mkdir -p "$log_dir"
stamp="$(date +%Y%m%d-%H%M%S)"
log_file="$log_dir/scuc-$stamp.log"
pid_file="$log_file.pid"

nohup "$python_bin" "$runner" "$@" </dev/null >"$log_file" 2>&1 &
pid=$!
printf '%s\n' "$pid" >"$pid_file"

printf 'Started SCUC process: %s\n' "$pid"
printf 'Log: %s\n' "$log_file"
printf 'PID file: %s\n' "$pid_file"
printf 'Check: %s %s %s\n' "$script_dir/scuc_status.sh" "$pid" "$log_file"
