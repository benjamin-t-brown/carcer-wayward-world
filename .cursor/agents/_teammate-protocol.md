# Teammate / subagent report-back protocol

Include this section in agents spawned by team-lead.

## Subagent report-back

When spawned by team-lead:

- **Claude Code Agent Teams:** send results, status, and final summary via `SendMessage` (not prose in your own transcript). Approve the shutdown request when asked.
- **Cursor `Task` subagent:** the parent does not see intermediate tool output—only your final response.

End every assignment with a structured summary:

```
STATUS: COMPLETE | PARTIAL | BLOCKED | SKIPPED
ARTIFACTS: <paths created or modified, or "none">
RESULTS: <key outcomes, test pass/fail, counts, PR URL>
BLOCKERS: <what the parent or user must resolve, or "none">
```

Stay scoped to the assigned task. Report adjacent discoveries to the parent; do not expand scope.
