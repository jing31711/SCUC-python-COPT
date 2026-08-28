#!/usr/bin/env bash
# Show process state, recent solver output, and the run summary when available.
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "Usage: $0 PID LOG_FILE" >&2
    exit 2
fi

pid="$1"
log_file="$2"

if ! [[ "$pid" =~ ^[0-9]+$ ]]; then
    echo "PID must be numeric: $pid" >&2
    exit 2
fi

if kill -0 "$pid" 2>/dev/null; then
    ps -p "$pid" -o pid= -o etime= -o command=
else
    echo "Process $pid is not running."
fi

if [[ -f "$log_file" ]]; then
    echo
    echo "Recent log output:"
    tail -n 40 "$log_file"

    run_dir="$(sed -n 's/^SCUC run directory: //p' "$log_file" | tail -n 1)"
    if [[ -n "$run_dir" && -f "$run_dir/summary.txt" ]]; then
        echo
        echo "Run summary:"
        cat "$run_dir/summary.txt"
    fi
else
    echo "Log file not found: $log_file" >&2
    exit 1
fi
