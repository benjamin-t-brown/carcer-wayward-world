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
for n in [5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65]:
    if n > len(chunks):
        n = len(chunks)
    body = ''.join('// --- from ' + c for c in chunks[:n])
    if '} // export' in body:
        body = body.rsplit('} // export', 1)[0]
    out = pre.replace('export module carcer.core;', f'export module carcer.core_n{n};')
    out = out + 'export {\n\n' + body + '\n} // export\n'
    Path(f'modules/carcer.core_n{n}.cppm').write_text(out, encoding='utf-8', newline='\n')
    print('wrote', n)
PY

mkdir -p .carcer-bmi
# ensure lib BMI exists
g++ -Wall -std=c++23 -g -fmodules-ts -Imodules -Ilib/sdl2w/modules -Ilib/sdl2w/modules/bmin \
  -c modules/carcer.lib.cppm -o .carcer-bmi/carcer.lib.o

for n in 5 10 15 20 25 30 35 40 45 50 55 60 65; do
  echo "=== n=$n ==="
  rm -f "gcm.cache/carcer.core_n${n}.gcm"
  if ! g++ -Wall -std=c++23 -g -fmodules-ts -Imodules -Ilib/sdl2w/modules -Ilib/sdl2w/modules/bmin \
      -c "modules/carcer.core_n${n}.cppm" -o ".carcer-bmi/core_n${n}.o" 2> "/tmp/core_n${n}.err"; then
    echo "COMPILE FAIL"; tail -n 6 "/tmp/core_n${n}.err"; continue
  fi
  ls -la "gcm.cache/carcer.core_n${n}.gcm"
  # Use a symbol that exists early: model types from first chunks, or skip if too early
  cat > "_t_n${n}.cpp" <<EOF
import carcer.core_n${n};
int main() {
  // touch something likely present after a few headers
  return 0;
}
EOF
  # Also try forcing binding load via a known type if n large enough
  if [ "$n" -ge 15 ]; then
    cat > "_t_n${n}.cpp" <<EOF
import carcer.core_n${n};
int main() {
  model::Player* p = nullptr;
  (void)p;
  return 0;
}
EOF
  fi
  if [ "$n" -ge 40 ]; then
    cat > "_t_n${n}.cpp" <<EOF
import carcer.core_n${n};
int main() {
  db::Database* d = nullptr;
  (void)d;
  return 0;
}
EOF
  fi
  if g++ -std=c++23 -g -fmodules-ts -Imodules -Ilib/sdl2w/modules -Ilib/sdl2w/modules/bmin \
      -c "_t_n${n}.cpp" -o "_t_n${n}.o" 2> "/tmp/imp_n${n}.err"; then
    echo "IMPORT+USE OK"
  else
    echo "IMPORT+USE FAIL"
    tail -n 8 "/tmp/imp_n${n}.err"
  fi
done
