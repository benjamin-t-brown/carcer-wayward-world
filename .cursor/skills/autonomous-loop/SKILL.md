---
name: autonomous-loop
description: >
  Autonomous task loop that implements all tasks from tasks.md hands-off with session management and stall detection. Use when user says "run autonomously", "execute the spec", "ralph loop", "implement all tasks", or "run unattended". Do NOT use for planning (use the planner agent) or interactive single-task execution (spawn cpp-expert / ceditor-expert via team-lead).
---

# Autonomous Loop (ralphloop)

Autonomous spec-driven task loop for Cursor. Implements approved tasks one at a time with intelligent session management, stall detection, and full observability.

## When to Use

- User has an approved `tasks.md` with `- [ ]` checklist items
- User wants hands-off execution (not interactive step-by-step)
- Tasks are well-scoped (each completable in under 50 turns)
- The spec directory exists at `.ai/specs/<feature>/`

## Prerequisites

Before running, verify:
1. A spec directory exists with at least a `tasks.md` containing `- [ ]` items
2. `jq` is installed (`brew install jq`, `apt-get install jq`, or MSYS2: `pacman -S jq`)
3. The user is on the correct branch and working directory is clean

## Script Location

Scripts ship with this skill at `.cursor/skills/autonomous-loop/scripts/`. Resolve from the repo root:

```bash
RALPHLOOP_DIR=".cursor/skills/autonomous-loop/scripts"
```

On Windows PowerShell, prefer the `.ps1` wrappers. For bash via MSYS2 UCRT64:

```powershell
.\scripts\Invoke-Ucrt64.ps1 ".cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> check"
```

## Running the Loop

Unlike Claude Code's `claude -p` orchestration, **you (the Cursor agent) are the loop executor**. Scripts manage state, prompts, logging, and stall detection; you implement each task (directly or by spawning `cpp-expert` / `ceditor-expert` per `.ai/specs/<feature>/routing.md`) and call the scripts between iterations.

### 1. Verify readiness

```bash
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> check
```

PowerShell:

```powershell
.cursor/skills/autonomous-loop/scripts/run.ps1 .ai/specs/<feature> check
```

### 2. Initialize (first run or fresh start)

```bash
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> init
```

### 3. Autonomous loop

Repeat until `should-continue` exits non-zero:

```bash
# Get the iteration prompt (read output and follow it)
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> next-prompt

# After implementing the task and marking it [x] in tasks.md:
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> record-progress --completed true --turns <N>

# Check whether to continue (0=continue, 1=done, 2=stall abort)
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> should-continue
echo $?
```

### With options

```bash
# Custom turn and stall limits
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> init --max-turns 40 --stall-limit 5

# Custom prompt template
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> next-prompt --prompt ./custom-prompt.md

# Resume after interruption
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> init --resume

# Pipe-friendly (no color codes)
.cursor/skills/autonomous-loop/scripts/run.sh .ai/specs/<feature> status --no-color
```

### Options reference

| Flag               | Default    | Description                            |
|--------------------|------------|----------------------------------------|
| `--max-turns N`    | 50         | Max turns per session before handoff   |
| `--stall-limit N`  | 5          | Consecutive stalls before abort        |
| `--prompt FILE`    | (embedded) | Custom prompt template                 |
| `--resume`         | off        | Resume from last completed iteration   |
| `--no-color`       | off        | Disable colored terminal output        |

## How It Works

1. Counts unchecked tasks (`- [ ]`) in the spec directory
2. Builds a prompt with the full spec and instructions via `next-prompt`
3. You implement the task, mark it `[x]` in `tasks.md`, and call `record-progress`
4. Logs each iteration under `.ai/specs/<feature>/.ralphloop/logs/`
5. Tracks turns per session; when over `--max-turns`, write handoff to `.ralphloop/handoff.md` and continue with fresh context (read handoff + spec, do not re-read full history)
6. Detects stalls (same task attempted repeatedly without completion) and aborts at `--stall-limit`
7. Repeats until all tasks are checked or stall limit hit

## Session Handoff

When approaching `--max-turns` in one session:

1. Write `.ralphloop/handoff.md` with: completed tasks, current task, blockers, files touched, next steps
2. Call `record-progress --handoff` (increments session counter)
3. Continue loop using handoff + spec files only — treat as a fresh session

## Post-Run Analysis

After the loop completes (or is interrupted):

```bash
# Summary report: iterations, stalls, tasks remaining
.cursor/skills/autonomous-loop/scripts/summarize.sh .ai/specs/<feature>

# Replay specific iterations for debugging
.cursor/skills/autonomous-loop/scripts/replay.sh .ai/specs/<feature> --from 5 --to 8
.cursor/skills/autonomous-loop/scripts/replay.sh .ai/specs/<feature> --iter 12
```

## Integration with team-lead workflow

Ralph Loop is the **autonomous alternative** to interactive expert batches in Phase 3 (Implementation):

| Mode | How | Supervision |
|------|-----|-------------|
| Interactive | `team-lead` spawns `cpp-expert` / `ceditor-expert` per task batch | User reviews after each batch |
| Autonomous | `team-lead` follows this skill (or user invokes it directly) | Hands-off until all tasks complete or stall abort |

After `planner` produces an approved `tasks.md`, `team-lead` asks:

> "Tasks approved. Would you like to run implementation **autonomously** (Ralph Loop — hands-off) or **interactively** (`cpp-expert` / `ceditor-expert` batches with review)?"

If autonomous: follow this skill end-to-end, report a structured summary when done.

## Guidance for the Assistant

When the user wants to run autonomously:

1. **Verify readiness**: Check that `.ai/specs/<feature>/tasks.md` exists and has unchecked items
2. **Check environment**: Run `check`; confirm branch, clean working tree, jq installed
3. **Initialize**: Run `init` (or `init --resume` if resuming)
4. **Execute the loop**: `next-prompt` → implement (or spawn `cpp-expert` / `ceditor-expert` when routing.md says so) → mark `[x]` → `record-progress` → `should-continue` until done
5. **After completion**: Run `summarize.sh`, analyze results, report to user

Do not stop between tasks for user approval unless blocked or stall-aborted.

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Stall abort after 5 iterations | Tasks too broad or ambiguous | Split large tasks, add acceptance criteria |
| Hitting max-turns every task | Tasks too large | Split tasks; lower scope per `- [ ]` item |
| Permission denials | Tool restrictions | Adjust Cursor permissions or narrow task scope |
| "No unchecked tasks" immediately | All tasks already `[x]` | Verify tasks.md has `- [ ]` items |
| Script not found | Skill not in repo | Ensure `.cursor/skills/autonomous-loop/` exists |

See [reference.md](reference.md) for cost expectations and detailed troubleshooting.

## Cost Expectations

| Scenario | Estimated Cost |
|----------|---------------|
| 10 tasks, good handoff discipline | $2-4 |
| 10 tasks, frequent full-context re-reads | $5-8 |
| 20 tasks, mixed | $8-15 |
| Complex tasks hitting max-turns | $15-25 |

Keep tasks small and specs concise. Session handoffs save context vs re-reading full conversation history.
