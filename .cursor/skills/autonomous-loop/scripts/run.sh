#!/usr/bin/env bash
# Autonomous loop state manager for Cursor agents.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=lib/common.sh
source "$SCRIPT_DIR/lib/common.sh"

usage() {
  cat <<'EOF'
Usage: run.sh <spec-dir> <command> [options]

Commands:
  check           Validate prerequisites
  init            Initialize .ralphloop state
  status          Show loop status
  next-prompt     Print iteration prompt to stdout
  record-progress Log iteration (--completed true|false, --turns N, --handoff)
  should-continue Exit 0=continue, 1=done, 2=stall abort

Options:
  --max-turns N     Max turns per session (default: 50)
  --stall-limit N   Consecutive stalls before abort (default: 5)
  --prompt FILE     Custom prompt template
  --resume          Resume existing state (init only)
  --no-color        Disable colored output
EOF
}

if [[ $# -lt 2 ]]; then
  usage
  exit 1
fi

SPEC_RAW="$1"
shift
SPEC="$(spec_dir_abs "$SPEC_RAW")"

remaining=("$@")
if [[ ${#remaining[@]} -lt 1 ]]; then
  usage
  exit 1
fi

CMD="${remaining[0]}"
remaining=("${remaining[@]:1}")
remaining=($(parse_common_flags "${remaining[@]}"))

record_progress() {
  local completed=0 turns=0 handoff=0
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --completed)
        [[ "$2" == "true" ]] && completed=1
        shift 2 ;;
      --turns)
        turns="$2"; shift 2 ;;
      --handoff)
        handoff=1; shift ;;
      *)
        err "Unknown record-progress flag: $1"
        exit 1 ;;
    esac
  done

  local sf state now task th iteration stall_count session turns_sess max_turns
  sf="$(state_file "$SPEC")"
  state="$(load_state "$sf")"
  now="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  task="$(first_unchecked_text "$SPEC")"
  th="$(task_hash "$task")"
  iteration="$(echo "$state" | jq -r '.iteration')"
  stall_count="$(echo "$state" | jq -r '.stall_count')"
  session="$(echo "$state" | jq -r '.session')"
  turns_sess="$(echo "$state" | jq -r '.turns_this_session')"
  max_turns="$(echo "$state" | jq -r '.max_turns')"

  local last_hash
  last_hash="$(echo "$state" | jq -r '.last_task_hash')"

  iteration=$((iteration + 1))
  turns_sess=$((turns_sess + turns))

  if [[ "$handoff" -eq 1 ]]; then
    session=$((session + 1))
    turns_sess=0
  fi

  if [[ "$completed" -eq 1 ]]; then
    stall_count=0
    task="$(first_unchecked_text "$SPEC")"
    th="$(task_hash "${task:-done}")"
  else
    if [[ "$th" == "$last_hash" && -n "$last_hash" ]]; then
      stall_count=$((stall_count + 1))
    else
      stall_count=0
    fi
  fi

  local log_dir log_file
  log_dir="$(ralph_dir "$SPEC")/logs"
  mkdir -p "$log_dir"
  log_file="$log_dir/iter-$(printf '%03d' "$iteration").json"

  jq -n \
    --arg timestamp "$now" \
    --argjson iteration "$iteration" \
    --argjson session "$session" \
    --arg task "$task" \
    --arg task_hash "$th" \
    --argjson completed "$completed" \
    --argjson turns "$turns" \
    --argjson handoff "$handoff" \
  '{
    timestamp: $timestamp,
    iteration: $iteration,
    session: $session,
    task: $task,
    task_hash: $task_hash,
    completed: ($completed == 1),
    turns: $turns,
    handoff: ($handoff == 1)
  }' > "$log_file"

  write_state "$sf" "$(echo "$state" | jq \
    --arg last_updated "$now" \
    --arg last_task "$task" \
    --arg last_task_hash "$th" \
    --argjson iteration "$iteration" \
    --argjson session "$session" \
    --argjson stall_count "$stall_count" \
    --argjson turns_this_session "$turns_sess" \
    '.iteration = $iteration
     | .session = $session
     | .stall_count = $stall_count
     | .turns_this_session = $turns_this_session
     | .last_task = $last_task
     | .last_task_hash = $last_task_hash
     | .last_updated = $last_updated')"

  printf '%s\n' "$(date -u +"%Y-%m-%dT%H:%M:%SZ") iter=$iteration completed=$completed stalls=$stall_count" \
    >> "$(ralph_dir "$SPEC")/progress.log"

  if [[ "$completed" -eq 1 ]]; then
    ok "Recorded iteration $iteration (completed)"
  else
    warn "Recorded iteration $iteration (not completed, stalls=$stall_count)"
  fi
}

should_continue() {
  local unchecked stall_limit stall_count
  unchecked="$(count_unchecked "$SPEC")"
  if [[ "$unchecked" -eq 0 ]]; then
    ok "All tasks complete"
    exit 1
  fi

  local sf state
  sf="$(state_file "$SPEC")"
  [[ -f "$sf" ]] || { err "State not initialized"; exit 2; }
  state="$(load_state "$sf")"
  stall_limit="$(echo "$state" | jq -r '.stall_limit')"
  stall_count="$(echo "$state" | jq -r '.stall_count')"

  if [[ "$stall_count" -ge "$stall_limit" ]]; then
    err "Stall limit reached ($stall_count >= $stall_limit)"
    exit 2
  fi

  info "$unchecked task(s) remaining"
  exit 0
}

print_status() {
  local sf unchecked
  if [[ ! -f "$(state_file "$SPEC")" ]]; then
    warn "Not initialized"
    unchecked="$(count_unchecked "$SPEC")"
    info "Unchecked tasks: $unchecked"
    return
  fi
  local state
  state="$(load_state "$(state_file "$SPEC")")"
  unchecked="$(count_unchecked "$SPEC")"
  info "Feature: $(echo "$state" | jq -r '.feature')"
  info "Iteration: $(echo "$state" | jq -r '.iteration')"
  info "Session: $(echo "$state" | jq -r '.session')"
  info "Unchecked: $unchecked"
  info "Stalls: $(echo "$state" | jq -r '.stall_count') / $(echo "$state" | jq -r '.stall_limit')"
  info "Turns this session: $(echo "$state" | jq -r '.turns_this_session') / $(echo "$state" | jq -r '.max_turns')"
  local cur
  cur="$(first_unchecked_text "$SPEC")"
  [[ -n "$cur" ]] && info "Current: $cur"
}

case "$CMD" in
  check)
    check_prerequisites "$SPEC"
    ;;
  init)
    init_state "$SPEC"
    ;;
  status)
    print_status
    ;;
  next-prompt)
    render_prompt "$SPEC"
    ;;
  record-progress)
    record_progress "${remaining[@]}"
    ;;
  should-continue)
    should_continue
    ;;
  *)
    err "Unknown command: $CMD"
    usage
    exit 1
    ;;
esac
