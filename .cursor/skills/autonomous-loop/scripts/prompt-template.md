# Autonomous Loop — Iteration Prompt

You are executing task **{{ITERATION}}** of an autonomous Ralph Loop for feature `{{FEATURE}}`.

## Spec Directory

`{{SPEC_DIR}}`

## Current Task (do only this)

{{CURRENT_TASK}}

## Remaining Tasks

{{REMAINING_COUNT}} unchecked item(s) after this one.

## Full tasks.md

```markdown
{{TASKS_CONTENT}}
```

## Additional Spec Files

{{SPEC_FILES}}

## Session Context

- Session: {{SESSION}}
- Turns this session: {{TURNS_THIS_SESSION}} / {{MAX_TURNS}}
- Stall count: {{STALL_COUNT}} / {{STALL_LIMIT}}

{{HANDOFF_SECTION}}

## Instructions

1. Implement **only** the current task. Do not skip ahead.
2. If `.ai/specs/<feature>/routing.md` assigns an owner (`cpp-expert`, `ceditor-expert`), spawn that agent via `Task` instead of editing code yourself.
3. Mark the task `- [x]` in `tasks.md` when complete.
4. Run tests/build if the task or spec requires it.
5. Do not ask for user approval — proceed autonomously unless truly blocked.
6. If blocked, leave the task unchecked and document the blocker in your final message.
7. When done, report: what changed, files touched, and whether the task is complete.

## Acceptance

Task is complete when the corresponding `- [ ]` line in `tasks.md` is marked `- [x]` with the same task text.
