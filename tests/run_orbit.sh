#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
PYTHON=${PYTHON:-python3}
CC=${CC:-clang}
mkdir -p build/gen build/host
"$PYTHON" tools/gen_tables.py build/gen/felucca_tables.h
"$PYTHON" tools/gen_fm6_patches.py build/gen/felucca_fm6.h
"$PYTHON" tools/gen_samples.py build/gen/felucca_samples.h
"$CC" -O1 -w -Ibuild/gen -Ifirmware/src tests/orbit_test.c -lm -o build/host/orbit_test
build/host/orbit_test build/orbit-demo.wav
"$CC" -O1 -w -DFELUCCA_CPU2=0 -Ibuild/gen -Ifirmware/src tests/orbit_test.c -lm -o build/host/orbit_single
build/host/orbit_single
"$CC" -O1 -w -fsanitize=address -Ibuild/gen -Ifirmware/src tests/orbit_test.c -lm -o build/host/orbit_asan
build/host/orbit_asan
