#!/usr/bin/env bash
# One-click classroom demo for partial-pivoting Doolittle decomposition.
#
#   ./run_demo.sh                 # compile and run every case in demo/
#   ./run_demo.sh demo/file.txt   # run one (or several) specific input file(s)
#
# Input file format (reads from stdin):
#   n                 number of rows/columns
#   n*n numbers       the matrix A, row by row
#   1 and n numbers   optional: also solve A*x=b for the given b
#   0 or nothing      factorization only
set -euo pipefail
cd "$(dirname "$0")"

CXX="${CXX:-g++}"
SRC="doolittle_partial_pivot.cpp"
BIN="build/doolittle_demo"

mkdir -p build
echo "==> Compiling ${SRC} with ${CXX} ..."
"${CXX}" -std=c++17 -O2 -Wall -Wextra -o "${BIN}" "${SRC}"
echo "==> Build OK"
echo

run_case() {
    local file="$1"
    echo "################################################################"
    echo "# ${file}"
    echo "################################################################"
    echo "----- input -----"
    cat "${file}"
    echo "----- program output -----"
    "${BIN}" < "${file}"
    echo
}

if [ "$#" -ge 1 ]; then
    cases=("$@")
else
    shopt -s nullglob
    cases=(demo/*.txt)
    if [ "${#cases[@]}" -eq 0 ]; then
        echo "No demo/*.txt input files found." >&2
        exit 1
    fi
fi

for f in "${cases[@]}"; do
    run_case "${f}"
done

echo "==> All demo cases finished."
