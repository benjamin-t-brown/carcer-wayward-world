#!/usr/bin/env bash
# Summarize autonomous loop run.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib/common.sh
source "$SCRIPT_DIR/lib/common.sh"

if [[ $# -lt 1 ]]; then
  err "Usage: summarize.sh <spec-dir>"
  exit 1
fi

SPEC="$(spec_dir_abs "$1")"
RALPH="$(ralph_dir "$SPEC")"
SF="$(state_file "$SPEC")"

echo "=== Ralph Loop Summary ==="
echo "Spec: $SPEC"
echo ""

unchecked="$(count_unchecked "$SPEC")"
echo "Unchecked tasks: $unchecked"

if [[ ! -f "$SF" ]]; then
  warn "No state file (loop never initialized)"
  exit 0
fi

state="$(load_state "$SF")"
iteration="$(echo "$state" | jq -r '.iteration')"
session="$(echo "$state" | jq -r '.session')"
stall_count="$(echo "$state" | jq -r '.stall_count')"
started="$(echo "$state" | jq -r '.started_at')"
updated="$(echo "$state" | jq -r '.last_updated')"

echo "Iterations: $iteration"
echo "Sessions: $session"
echo "Stall count: $stall_count"
echo "Started: $started"
echo "Last updated: $updated"
echo ""

LOG_DIR="$RALPH/logs"
if [[ -d "$LOG_DIR" ]]; then
  completed=0
  incomplete=0
  total_turns=0
  for f in "$LOG_DIR"/iter-*.json; do
    [[ -f "$f" ]] || continue
    if jq -e '.completed == true' "$f" >/dev/null 2>&1; then
      completed=$((completed + 1))
    else
      incomplete=$((incomplete + 1))
    fi
    t="$(jq -r '.turns // 0' "$f")"
    total_turns=$((total_turns + t))
  done
  echo "Completed iterations: $completed"
  echo "Incomplete iterations: $incomplete"
  echo "Total turns logged: $total_turns"
  echo ""

  echo "=== Recent iterations ==="
  ls -1 "$LOG_DIR"/iter-*.json 2>/dev/null | tail -5 | while read -r f; do
    jq -r '"\(.iteration): \(.task) [completed=\(.completed) turns=\(.turns)]"' "$f"
  done
fi

if [[ -f "$RALPH/progress.log" ]]; then
  echo ""
  echo "=== Progress log (last 10) ==="
  tail -10 "$RALPH/progress.log"
fi

if [[ "$stall_count" -ge "$(echo "$state" | jq -r '.stall_limit')" ]]; then
  echo ""
  warn "RUNAWAY: Stall limit was reached"
fi

if [[ "$unchecked" -gt 0 && "$iteration" -gt 0 ]]; then
  avg_turns=$((total_turns / iteration))
  if [[ "$avg_turns" -gt 30 ]]; then
    warn "High average turns per iteration ($avg_turns) — consider smaller tasks"
  fi
fi
