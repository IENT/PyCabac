#!/usr/bin/env bash

set -euo pipefail

git rev-parse --git-dir 1>/dev/null
readonly git_root_dir="$(git rev-parse --show-toplevel)"

readonly build_dir="${git_root_dir}/build"

mkdir -p "${build_dir}"
cd "${build_dir}"
cmake .. -DCMAKE_BUILD_TYPE=Release
make --jobs
