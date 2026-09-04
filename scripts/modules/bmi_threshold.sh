#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../../src"

python - <<'PY'
from pathlib import Path
import re
p = Path('modules/carcer.core.cppm')
t = p.read_text(encoding='utf-8')
# Change module name helper
pre, rest = t.split('export {', 1)
parts = rest.split('// --- from ')
chunks = parts[1:]
print('chunks', len(chunks))
for n in [10, 20, 30, 40, 50, 60, 65]:
    body = ''.join('// --- from ' + c for c in chunks[:n])
    out = pre.replace('export module carcer.core;', f'export module carcer.core_n{n};')
    out = out + 'export {\n\n' + body + '\n} // export\n'
    Path(f'modules/carcer.core_n{n}.cppm').write_text(out, encoding='utf-8', newline='\n')
    print('wrote', n, 'bytes', len(out))
PY

mkdir -p .carcer-bmi
for n in 10 20 30 40 50 60 65; do
  echo "=== n=$n ==="
  rm -f "gcm.cache/carcer.core_n${n}.gcm"
  if ! g++ -Wall -std=c++23 -g -fmodules-ts -Imodules -Ilib/sdl2w/modules -Ilib/sdl2w/modules/bmin \
      -c "modules/carcer.core_n${n}.cppm" -o ".carcer-bmi/core_n${n}.o" 2> "/tmp/core_n${n}.err"; then
    echo "COMPILE FAIL"
    tail -n 8 "/tmp/core_n${n}.err"
    continue
  fi
  ls -la "gcm.cache/carcer.core_n${n}.gcm"
  cat > "_t_n${n}.cpp" <<EOF
import carcer.core_n${n};
int main() { return 0; }
EOF
  if g++ -std=c++23 -g -fmodules-ts -Imodules -Ilib/sdl2w/modules -Ilib/sdl2w/modules/bmin \
      -c "_t_n${n}.cpp" -o "_t_n${n}.o" 2> "/tmp/imp_n${n}.err"; then
    echo "IMPORT OK"
  else
    echo "IMPORT FAIL"
    tail -n 8 "/tmp/imp_n${n}.err"
  fi
done
