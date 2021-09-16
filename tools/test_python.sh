#!/usr/bin/env bash

set -euo pipefail

git rev-parse --git-dir 1>/dev/null
readonly git_root_dir="$(git rev-parse --show-toplevel)"

(
    cd "${git_root_dir}"
    python3 -m unittest discover --verbose
)
