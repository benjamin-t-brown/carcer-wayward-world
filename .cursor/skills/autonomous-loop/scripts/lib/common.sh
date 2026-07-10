#!/usr/bin/env bash
# Shared helpers for autonomous-loop scripts.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SKILL_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
DEFAULT_PROMPT="$SKILL_DIR/prompt-template.md"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

NO_COLOR=0
MAX_TURNS=50
STALL_LIMIT=5
RESUME=0
CUSTOM_PROMPT=""

color() {
  local c="$1"; shift
  if [[ "$NO_COLOR" -eq 1 ]]; then
    printf '%s\n' "$*"
  else
    printf "${c}%s${NC}\n" "$*"
  fi
}

info()  { color "$CYAN" "$*"; }
ok()    { color "$GREEN" "$*"; }
warn()  { color "$YELLOW" "$*"; }
err()   { color "$RED" "$*" >&2; }

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    err "Required command not found: $1"
    exit 1
  fi
}

spec_dir_abs() {
  local d="$1"
  cd "$d" && pwd
}

ralph_dir() {
  echo "$1/.ralphloop"
}

state_file() {
  echo "$(ralph_dir "$1")/state.json"
}

tasks_file() {
  echo "$1/tasks.md"
}

count_unchecked() {
  local tf
  tf="$(tasks_file "$1")"
  if [[ ! -f "$tf" ]]; then
    echo 0
    return
  fi
  grep -cE '^- \[ \]' "$tf" 2>/dev/null || true
}

first_unchecked_line() {
  local tf
  tf="$(tasks_file "$1")"
  grep -m1 -E '^- \[ \]' "$tf" 2>/dev/null || true
}

first_unchecked_text() {
  local line
  line="$(first_unchecked_line "$1")"
  echo "$line" | sed -E 's/^- \[ \] //'
}

task_hash() {
  printf '%s' "$1" | sha256sum | cut -c1-12
}

load_state() {
  local sf="$1"
  if [[ ! -f "$sf" ]]; then
    err "State not found: $sf (run init first)"
    exit 1
  fi
  cat "$sf"
}

write_state() {
  local sf="$1"
  local json="$2"
  mkdir -p "$(dirname "$sf")"
  printf '%s\n' "$json" | jq '.' > "$sf"
}

init_state() {
  local spec="$1"
  local sf
  sf="$(state_file "$spec")"
  local now feature
  now="$(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  feature="$(basename "$spec")"

  if [[ "$RESUME" -eq 1 && -f "$sf" ]]; then
    ok "Resuming existing state: $sf"
    return 0
  fi

  mkdir -p "$(ralph_dir "$spec")/logs"
  write_state "$sf" "$(jq -n \
    --arg feature "$feature" \
    --arg started_at "$now" \
    --arg last_updated "$now" \
    --argjson max_turns "$MAX_TURNS" \
    --argjson stall_limit "$STALL_LIMIT" \
    '{
      feature: $feature,
      iteration: 0,
      session: 1,
      stall_count: 0,
      last_task: "",
      last_task_hash: "",
      max_turns: $max_turns,
      stall_limit: $stall_limit,
      turns_this_session: 0,
      started_at: $started_at,
      last_updated: $last_updated
    }')"
  ok "Initialized: $sf"
}

check_prerequisites() {
  local spec="$1"
  require_cmd jq
  require_cmd git

  if [[ ! -d "$spec" ]]; then
    err "Spec directory not found: $spec"
    exit 1
  fi

  local tf
  tf="$(tasks_file "$spec")"
  if [[ ! -f "$tf" ]]; then
    err "tasks.md not found: $tf"
    exit 1
  fi

  local unchecked
  unchecked="$(count_unchecked "$spec")"
  if [[ "$unchecked" -eq 0 ]]; then
    warn "No unchecked tasks in $tf"
    exit 1
  fi

  if [[ -n "$(git status --porcelain 2>/dev/null)" ]]; then
    warn "Working tree is not clean"
    git status --short
    exit 1
  fi

  ok "Ready: $unchecked unchecked task(s) in $spec"
}

read_spec_files_section() {
  local spec="$1"
  local out=""
  for f in requirements.md design.md acceptance.md; do
    if [[ -f "$spec/$f" ]]; then
      out+=$'\n'"### $f"$'\n\n'"$(cat "$spec/$f")"$'\n'
    fi
  done
  if [[ -z "$out" ]]; then
    echo "(No additional spec files found.)"
  else
    printf '%s' "$out"
  fi
}

render_prompt() {
  local spec="$1"
  local sf state iteration session turns max_turns stall_count stall_limit
  local current_task remaining handoff_section tasks_content spec_files
  local template prompt_file feature

  sf="$(state_file "$spec")"
  state="$(load_state "$sf")"
  iteration="$(echo "$state" | jq -r '.iteration')"
  session="$(echo "$state" | jq -r '.session')"
  turns="$(echo "$state" | jq -r '.turns_this_session')"
  max_turns="$(echo "$state" | jq -r '.max_turns')"
  stall_count="$(echo "$state" | jq -r '.stall_count')"
  stall_limit="$(echo "$state" | jq -r '.stall_limit')"
  feature="$(echo "$state" | jq -r '.feature')"

  current_task="$(first_unchecked_text "$spec")"
  if [[ -z "$current_task" ]]; then
    err "No unchecked tasks remaining"
    exit 1
  fi

  remaining="$(count_unchecked "$spec")"
  remaining=$((remaining - 1))
  tasks_content="$(cat "$(tasks_file "$spec")")"
  spec_files="$(read_spec_files_section "$spec")"

  handoff_section=""
  local handoff_file
  handoff_file="$(ralph_dir "$spec")/handoff.md"
  if [[ -f "$handoff_file" ]]; then
    handoff_section=$'## Handoff from previous session\n\n'"$(cat "$handoff_file")"
  fi

  prompt_file="${CUSTOM_PROMPT:-$DEFAULT_PROMPT}"
  if [[ ! -f "$prompt_file" ]]; then
    err "Prompt template not found: $prompt_file"
    exit 1
  fi
  template="$(cat "$prompt_file")"

  template="${template//\{\{ITERATION\}\}/$((iteration + 1))}"
  template="${template//\{\{FEATURE\}\}/$feature}"
  template="${template//\{\{SPEC_DIR\}\}/$spec}"
  template="${template//\{\{CURRENT_TASK\}\}/$current_task}"
  template="${template//\{\{REMAINING_COUNT\}\}/$remaining}"
  template="${template//\{\{TASKS_CONTENT\}\}/$tasks_content}"
  template="${template//\{\{SPEC_FILES\}\}/$spec_files}"
  template="${template//\{\{SESSION\}\}/$session}"
  template="${template//\{\{TURNS_THIS_SESSION\}\}/$turns}"
  template="${template//\{\{MAX_TURNS\}\}/$max_turns}"
  template="${template//\{\{STALL_COUNT\}\}/$stall_count}"
  template="${template//\{\{STALL_LIMIT\}\}/$stall_limit}"
  template="${template//\{\{HANDOFF_SECTION\}\}/$handoff_section}"

  printf '%s\n' "$template"
}

parse_common_flags() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      --max-turns)
        MAX_TURNS="$2"; shift 2 ;;
      --stall-limit)
        STALL_LIMIT="$2"; shift 2 ;;
      --resume)
        RESUME=1; shift ;;
      --no-color)
        NO_COLOR=1; shift ;;
      --prompt)
        CUSTOM_PROMPT="$2"; shift 2 ;;
      *)
        break ;;
    esac
  done
  echo "$@"
}
