#!/usr/bin/env bash
# Replay iteration logs from an autonomous loop run.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib/common.sh
source "$SCRIPT_DIR/lib/common.sh"

usage() {
  cat <<'EOF'
Usage: replay.sh <spec-dir> [options]

Options:
  --iter N       Replay single iteration
  --from N       Start iteration (inclusive)
  --to N         End iteration (inclusive)
  --no-color     Disable colored output
EOF
}

SPEC_RAW=""
FROM=""
TO=""
ITER=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --iter) ITER="$2"; shift 2 ;;
    --from) FROM="$2"; shift 2 ;;
    --to) TO="$2"; shift 2 ;;
    --no-color) NO_COLOR=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *)
      if [[ -z "$SPEC_RAW" ]]; then
        SPEC_RAW="$1"; shift
      else
        err "Unknown argument: $1"
        usage
        exit 1
      fi
      ;;
  esac
done

if [[ -z "$SPEC_RAW" ]]; then
  usage
  exit 1
fi

SPEC="$(spec_dir_abs "$SPEC_RAW")"
LOG_DIR="$(ralph_dir "$SPEC")/logs"

if [[ ! -d "$LOG_DIR" ]]; then
  err "No logs found: $LOG_DIR"
  exit 1
fi

replay_file() {
  local n="$1"
  local f="$LOG_DIR/iter-$(printf '%03d' "$n").json"
  if [[ ! -f "$f" ]]; then
    warn "Missing: $f"
    return
  fi
  echo "--- Iteration $n ---"
  jq '.' "$f"
  echo ""
}

if [[ -n "$ITER" ]]; then
  replay_file "$ITER"
  exit 0
fi

FROM="${FROM:-1}"
TO="${TO:-$FROM}"

if [[ "$TO" -lt "$FROM" ]]; then
  err "--to must be >= --from"
  exit 1
fi

for ((i=FROM; i<=TO; i++)); do
  replay_file "$i"
done
