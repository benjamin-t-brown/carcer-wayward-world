---
name: feature-dev
description: >
  Orchestrates team-lead through the full spec-driven feature pipeline: plan,
  implement, and test. Use when the user says "build a feature", "plan implement
  and test", "feature dev", "run the full workflow", "develop X end-to-end", or
  invokes feature-dev. Spawns planner, domain experts, and quality checks via
  team-lead. Do NOT use for single-phase requests (plan only, debug only, ship
  only) or when the user already invoked autonomous-loop or a pre-bound
  /ai-dev-workflow:* command.
---

# Feature Dev (team-lead pipeline)

End-to-end feature development: **plan → implement → test**, coordinated by team-lead.

## Your role

You are the **session coordinator**. Read and follow `.cursor/agents/team-lead.md` for delegation rules, gates, and dashboards. You do not write code or tests yourself — you route, spawn specialists, and synthesize results for the user.

**Overrides for this skill** (apply on top of team-lead defaults):

| Default | feature-dev override |
|---|---|
| Routing may choose lightweight mode | **Always standard mode** — full pipeline |
| Quality step optional in lightweight | **Always run Quality + Fix** before done |
| User may skip planning on small tasks | **Never skip Planning** unless user explicitly says "skip planning" |

## Invocation

Extract from the user's message:

1. **Feature name** — kebab-case slug for `.ai/specs/<feature>/` (infer from description if omitted).
2. **Feature description** — what to build, constraints, success criteria.
3. **Resume hint** — if they say "continue", "resume", or name an existing spec dir, run [State detection](.cursor/agents/team-lead.md#state-detection) first and skip completed steps.

If the description is too vague to plan, ask **one** clarifying question, then proceed.

## Pipeline checklist

Track and report progress against this list:

```
Feature Dev Progress:
- [ ] 1. Setup — branch, spec dir, status dashboard
- [ ] 2. Planning — planner → requirements, design, tasks, acceptance, routing
- [ ] 3. Implementation — cpp-expert / ceditor-expert (interactive or autonomous-loop)
- [ ] 4. Quality + Fix — verify acceptance.md, automated review if requested
- [ ] 5. Summary — report artifacts, test results, suggested next steps (PR if applicable)
```

Emit the team-lead routing callout before each delegation:

```
> Routing: <size> / <phase> / <subject> -> <category> (standard mode, full pipeline)
```

## Step-by-step

### 1. Setup

1. Read `DEVELOPMENT.md` when present.
2. Present the [status dashboard](.cursor/agents/team-lead.md#state-detection) if resuming; otherwise show a fresh dashboard.
3. Suggest branch `feature/@<id>-<kebab-description>` for new work.
4. **Gate:** wait for user confirmation before spawning teammates.

### 2. Planning

Spawn `planner` via `Task`:

```
Task(
  subagent_type: "planner",
  prompt: "Feature: <name>
Description: <user description>

Create spec artifacts under .ai/specs/<feature>/:
requirements.md, design.md, tasks.md, acceptance.md, routing.md.

Ground in existing codebase patterns. Report back per .cursor/agents/_teammate-protocol.md."
)
```

**Gate:** user approves the plan (requirements, tasks, acceptance criteria).

### 3. Implementation

After plan approval, ask:

> Tasks approved. Run implementation **A. Interactive** (expert batches with review after each) or **B. Autonomous** (hands-off via autonomous-loop)?

| Choice | Action |
|---|---|
| **A. Interactive** | Spawn `cpp-expert` / `ceditor-expert` per `.ai/specs/<feature>/routing.md`, 1–2 tasks per batch |
| **B. Autonomous** | Follow `.cursor/skills/autonomous-loop/SKILL.md`; spawn experts per iteration when routing.md says so |

**Gate:** user reviews implementation before quality step.

### 4. Quality + Fix

1. Read `.ai/specs/<feature>/acceptance.md` and verify each criterion (build commands, tests, manual checks).
2. Offer automated review: spawn `bugbot` or `security-review` `Task` subagents on branch changes if the user wants.
3. Triage findings per team-lead [Fix Cycle](.cursor/agents/team-lead.md#fix-cycle); spawn domain experts for fixes.
4. Re-verify acceptance criteria after fixes.

**Gate:** user approves quality results.

### 5. Summary

Report:

- Spec path and task completion count
- Files touched (from teammate ARTIFACTS)
- Build/test outcomes
- Open blockers
- Suggested next step (open PR, manual QA, etc.)

PR prep and review are **out of scope** unless the user asks — point them to `gh` / ship workflow.

## Delegation rules

- Max 3–4 concurrent `Task` subagents.
- Every spawn prompt must include the report-back contract from `.cursor/agents/_teammate-protocol.md`.
- Prefer `cpp-expert` (`src/`), `ceditor-expert` (`ceditor/`), `planner` (specs) — never `generalPurpose` when a specialist matches.
- Nested subagent cannot re-delegate: if you were spawned via `Task` and lack `Task` access, emit the hard-block from `.cursor/skills/routing/references/hard-block-contract.md` and stop.

## Windows builds

When acceptance or experts need C++ builds, use MSYS2 UCRT64 per `.cursor/rules/windows-msys2-build.mdc`:

```powershell
.\scripts\Invoke-Ucrt64.ps1 "cd src && make -j8"
```

## Examples

See [examples.md](examples.md) for sample invocations and expected first responses.
