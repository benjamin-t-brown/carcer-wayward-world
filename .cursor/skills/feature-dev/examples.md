# feature-dev examples

## Example 1: New feature from scratch

**User:** Build a feature to add ability cooldown display in the asset editor.

**Coordinator first response:**

```
Feature: ability-cooldown-display
Branch: (not started — suggest feature/@42-ability-cooldown-display)

Feature Dev Progress:
- [ ] 1. Setup
- [ ] 2. Planning
- [ ] 3. Implementation
- [ ] 4. Quality + Fix
- [ ] 5. Summary

Suggested next action: confirm feature name and branch, then spawn planner.

Proceed with planning?
```

After confirmation → spawn `planner` → gate on plan approval → implementation choice → quality → summary.

---

## Example 2: Resume existing spec

**User:** Continue feature dev for ability-cooldown-display.

**Coordinator:** Run state detection on `.ai/specs/ability-cooldown-display/`, show dashboard with task counts, suggest next unchecked task or quality step.

---

## Example 3: Autonomous implementation

**User:** Plan implement and test status effect stacking — run implementation autonomously.

**Flow:** Planning gate → user picks **B. Autonomous** → follow `autonomous-loop` skill → quality step runs after all tasks `[x]`.

---

## Example 4: Out of scope (do not use this skill)

| User says | Use instead |
|---|---|
| "Just plan the billing flow" | Spawn `planner` directly or `/ai-dev-workflow:plan` |
| "Fix the loader bug in src" | team-lead lightweight routing → `cpp-expert` |
| "Run autonomous loop on tasks.md" | `.cursor/skills/autonomous-loop/SKILL.md` |
| "Ship this branch" | team-lead PR ship route |
