#!/usr/bin/env bash

set -euo pipefail

git rev-parse --git-dir 1>/dev/null
readonly git_root_dir="$(git rev-parse --show-toplevel)"

readonly tests="${git_root_dir}/build/tests/tests"

if [[ -x "${tests}" ]]; then
    "${tests}"
else
    echo "Error: File '${tests}' is not executable"
    echo "Hint: Did you build it?"
    exit 1
fi
