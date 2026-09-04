#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../../src"

python - <<'PY'
from pathlib import Path
p = Path('modules/carcer.core.cppm')
t = p.read_text(encoding='utf-8')
pre, rest = t.split('export {', 1)
chunks = rest.split('// --- from ')[1:]
print('total', len(chunks))
for n in range(60, len(chunks)+1):
    body = ''.join('// --- from ' + c for c in chunks[:n])
    # drop trailing export closer from last chunk if present
    if body.rstrip().endswith('} // export'):
        body = body.rsplit('} // export', 1)[0]
    out = pre.replace('export module carcer.core;', f'export module carcer.core_n{n};')
    out = out + 'export {\n\n' + body + '\n} // export\n'
    Path(f'modules/carcer.core_n{n}.cppm').write_text(out, encoding='utf-8', newline='\n')
    # show last chunk name
    name = chunks[n-1].split('\n', 1)[0]
    print(f'n={n} last={name.strip()} bytes={len(out)}')
PY

mkdir -p .carcer-bmi
for n in 60 61 62 63 64 65; do
  echo "=== n=$n ==="
  rm -f "gcm.cache/carcer.core_n${n}.gcm"
  if ! g++ -Wall -std=c++23 -g -fmodules-ts -Imodules -Ilib/sdl2w/modules -Ilib/sdl2w/modules/bmin \
      -c "modules/carcer.core_n${n}.cppm" -o ".carcer-bmi/core_n${n}.o" 2> "/tmp/core_n${n}.err"; then
    echo "COMPILE FAIL"; tail -n 10 "/tmp/core_n${n}.err"; continue
  fi
  ls -la "gcm.cache/carcer.core_n${n}.gcm"
  printf 'import carcer.core_n%s;\nint main(){return 0;}\n' "$n" > "_t_n${n}.cpp"
  if g++ -std=c++23 -g -fmodules-ts -Imodules -Ilib/sdl2w/modules -Ilib/sdl2w/modules/bmin \
      -c "_t_n${n}.cpp" -o "_t_n${n}.o" 2> "/tmp/imp_n${n}.err"; then
    echo "IMPORT OK"
  else
    echo "IMPORT FAIL"; tail -n 10 "/tmp/imp_n${n}.err"
  fi
done
