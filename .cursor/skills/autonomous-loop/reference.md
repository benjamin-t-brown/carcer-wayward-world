# Autonomous Loop — Reference

## Spec Directory Layout

```
.ai/specs/<feature>/
├── tasks.md           # Required: - [ ] checklist items
├── requirements.md    # Optional: full requirements
├── design.md          # Optional: design notes
└── .ralphloop/        # Created by init (gitignored recommended)
    ├── state.json
    ├── handoff.md
    ├── progress.log
    └── logs/
        ├── iter-001.json
        └── iter-002.json
```

## state.json Schema

```json
{
  "feature": "my-feature",
  "iteration": 3,
  "session": 1,
  "stall_count": 0,
  "last_task": "Implement login form",
  "last_task_hash": "abc123",
  "max_turns": 50,
  "stall_limit": 5,
  "turns_this_session": 12,
  "started_at": "2026-06-19T10:00:00Z",
  "last_updated": "2026-06-19T10:30:00Z"
}
```

## Stall Detection

A stall is recorded when `record-progress --completed false` is called for the same task hash consecutively. The task hash is derived from the first unchecked task line text. After `--stall-limit` consecutive stalls, `should-continue` exits code 2.

## Script Commands

| Command | Purpose |
|---------|---------|
| `check` | Validate prerequisites; exit 0 on success |
| `init` | Create `.ralphloop/` state; use `--resume` to continue |
| `status` | Print iteration, remaining tasks, stall count |
| `next-prompt` | Emit prompt for current iteration |
| `record-progress` | Log iteration; `--completed true/false`, `--turns N`, `--handoff` |
| `should-continue` | Exit 0=continue, 1=all done, 2=stall abort |

## External SDK Automation (Optional)

For CI or fully unattended runs outside the IDE, use the [Cursor SDK](https://cursor.com/docs/sdk) with `Agent.prompt()` per iteration instead of in-IDE execution. This skill's state files are compatible with either approach.

## Cost Tips

- One task per `- [ ]` item; each should be completable in one focused session
- Trim `requirements.md`; keep `tasks.md` as the execution source of truth
- Use handoff files instead of carrying full chat history across sessions
- Run `summarize.sh` after completion to spot runaway iteration counts
