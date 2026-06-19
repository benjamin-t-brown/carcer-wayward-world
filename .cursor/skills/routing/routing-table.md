# Routing table

Lookup rows by `category` after classifying with `decision-tree.md`.

Routes use workspace agents in `.cursor/agents/`. Drop the `ai-dev-workflow:` prefix when spawning via Cursor `Task`.

| category | route | requires_plugin | requires_mcp | examples | spawn-prompt |
|---|---|---|---|---|---|
| coding-feature-dev | planner | | | build user-auth; implement billing; add a feature | Plan the requested feature. Create persistent artifacts under `.ai/specs/<feature>/` (requirements.md, design.md, tasks.md, acceptance.md, routing.md). After plan approval, team-lead routes to cpp-expert / ceditor-expert or autonomous-loop. |
| coding-small-fix | cpp-expert | | | fix typo in src; rename C++ symbol; loader bug | Apply the requested small change under `src/`. Read DEVELOPMENT.md and AGENTS.md. Preserve behavior. Build and run relevant tests. |
| coding-small-fix-editor | ceditor-expert | | | fix editor form; typo in ceditor types | Apply the requested small change under `ceditor/`. Run `npm run build`. Preserve behavior. |
| coding-debugging | superpowers:systematic-debugging | superpowers | | why is the login page failing; debug this test | Debug the reported issue. Reproduce, isolate root cause, minimal fix, verify. If `superpowers` missing, spawn `cpp-expert` or `ceditor-expert` by failing path instead. |
| coding-explore | explore | | | how does auth work; where is X handled | Explore the codebase read-only. Return file paths, call flow, concise explanation. Do not change code. |
| coding-architecture | planner | | | design the billing flow; architecture for user-auth | Produce or extend design.md under `.ai/specs/<feature>/` for the requested scope. Read existing patterns first. |
| coding-review | bugbot | | | review this code; review my changes | Review the specified diff for correctness and maintainability. Return prioritized findings. |
| plan-driven-throwaway | planner | | | quick plan for tonight; plan this refactor | Produce a minimal spec under `.ai/specs/<feature>/` optimized for immediate execution; note if user wants throwaway vs persistent. |
| docs-authoring | planner | | | write a design doc for the new billing flow | Draft or extend design.md / requirements.md under `.ai/specs/<feature>/` or user-named doc path. |
| pr-ship | (none) | | | ship this; create a PR | No pr-preparer agent in workspace. Team-lead reports git/gh state read-only and asks user to open PR or approve explicit git commands. |

## Stack routing (implementation)

When executing `tasks.md`, prefer:

| Stack | Agent | Paths |
|---|---|---|
| C++ game | `cpp-expert` | `src/**/*.cpp`, `src/**/*.h` |
| Asset editor | `ceditor-expert` | `ceditor/**/*.ts(x)`, `src/assets/db/*.json` |

Autonomous full-spec runs: `.cursor/skills/autonomous-loop/SKILL.md` (team-lead coordinates; spawns experts per task).
