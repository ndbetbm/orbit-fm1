#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
PYTHON=${PYTHON:-python3}
mkdir -p build/gen
"$PYTHON" tools/gen_tables.py build/gen/felucca_tables.h
"$PYTHON" tools/gen_fm6_patches.py build/gen/felucca_fm6.h
"$PYTHON" tools/gen_samples.py build/gen/felucca_samples.h
"$PYTHON" tools/gen_aa_font.py build/gen/ui_fonts.h --preset inter-tight
"$PYTHON" tools/gen_aa_icons.py build/gen/ui_icons.h
"$PYTHON" tools/gen_aa_keycaps.py build/gen/ui_keycaps.h
"$PYTHON" tools/gen_ui_palettes.py build/gen/ui_palettes.h
sh web/emu/build.sh
${CC:-clang} -O2 -ffp-contract=off -w -Ibuild/gen -Ifirmware/src web/emu/orbit_native.c -lm -o build/emu/orbit_native
build/emu/orbit_native build/emu/orbit-native.f32
node web/emu/orbit_test.mjs build/emu/felucca.wasm build/emu/orbit-native.f32
node web/emu/scope_test.mjs build/emu/felucca.wasm
node web/emu/filter_test.mjs build/emu/felucca.wasm
node web/emu/random_test.mjs build/emu/felucca.wasm
node web/emu/shortcut_test.mjs build/emu/felucca.wasm
${CC:-clang} -O1 -w -fsanitize=address -Ibuild/gen -Ifirmware/src web/emu/envelope_test.c -lm -o build/emu/envelope_test
build/emu/envelope_test
${CC:-clang} -O1 -w -fsanitize=address -Ibuild/gen -Ifirmware/src web/emu/shortcut_test.c -lm -o build/emu/shortcut_test
build/emu/shortcut_test
cp web/emu/ORBIT-README.md build/emu/README.md
cp web/emu/serve.py web/emu/起動.command build/emu/
"$PYTHON" - <<'PY'
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED
root=Path.cwd()
with ZipFile(root/'build/emu/ORBIT-browser-source.zip','w',ZIP_DEFLATED) as z:
    for p in sorted(root.rglob('*')):
        rel=p.relative_to(root)
        if p.is_file() and not any(x in {'.git','.gitmodules','build','__pycache__'} for x in rel.parts):
            z.write(p,Path('ORBIT-source')/rel)
PY
