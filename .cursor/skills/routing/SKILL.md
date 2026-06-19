---
name: routing
description: >
  This skill should be used when team-lead needs to classify and route an ad-hoc
  user request to the correct plugin, skill, or agent. Use when the user says
  "build X", "fix Y", "review Z", or any freeform request that does not match a
  pre-bound /ai-dev-workflow:* command. Returns a route, a hard-block, or one
  disambiguation question. Do NOT use when the user explicitly invokes a
  /ai-dev-workflow:* command or names a specific skill by name.
---

# Routing

Decide how an ad-hoc user request gets handled: spawn a teammate, invoke a delegated skill, or stop and ask one question. Routing data lives in `routing-table.md`; classifier prose lives in `decision-tree.md`. This skill orchestrates the lookup; it does NOT define methodology.

## When to invoke

Invoke this skill on every ad-hoc team-lead request that does NOT match a pre-bound `/ai-dev-workflow:*` command (e.g. `/ai-dev-workflow:plan`, `/ai-dev-workflow:debug`, `/ai-dev-workflow:review`, `/ai-dev-workflow:ship`, `/ai-dev-workflow:doctor`, `/ai-dev-workflow:resume`). Pre-bound commands skip routing and go straight to their handler.

## Inputs

1. The raw user request (latest user message).
2. The SessionStart status block, available in context as `<!-- ai-dev-workflow:status ... -->`. Schema: `ai-dev-workflow.status.v1` (see `references/hook-output-schema.md`). If the block is absent, treat all `requires_plugin` and `requires_mcp` checks as `unknown` and prefer routes that have no deps.

## Algorithm (5 steps)

1. **Read status block.** Parse the JSON tail from `additionalContext`. Capture `plugins.ok`, `plugins.missing`, `plugins.unknown`, and the analogous `mcps` arrays.

2. **Classify the request.** Walk `decision-tree.md` to assign:
   - **Size**: trivial / small / medium / large.
   - **Phase**: explore / design / implement / test / review / debug / ship / oncall / docs.
   - **Subject**: code / spec / config / infra / incident / PR.
   The combination keys to a single category in the routing table.

3. **Look up the route.** Find the row in `routing-table.md` whose `category` matches the classified category, or whose `examples` cell most closely matches the user's phrasing. Do not match more than one row; if two are plausible, prefer the more specific category and note the alternative.

4. **Check deps against the status block.**
   - If `requires_plugin` is set and the plugin is in `plugins.missing` OR `plugins.unknown`: emit a hard-block per `references/hard-block-contract.md` (plugin missing variant). Do NOT fall back to a degraded methodology.
   - If `requires_mcp` is set and any listed MCP is in `mcps.missing` or `mcps.auth_needed`: emit a hard-block (MCP variant) with auth or install instructions.
   - If the status block is entirely absent (hook never ran or crashed) AND the route declares `requires_plugin` or `requires_mcp`: treat ALL declared deps as MISSING. Emit the hard-block message, or ask the user to run `/ai-dev-workflow:doctor` to populate the status block. NEVER optimistically assume a dep is available when no status data exists.
   - **`unknown` = MISSING for hard-block purposes.** A probe timeout or failure (which places a dep in `unknown`) is never treated as OK. The user must resolve the unknown state (reinstall, re-run doctor) before the route can proceed.

5. **Decide.** Before invoking the chosen route, emit a single-line routing classification message so the decision is observable and debuggable:
   ```
   > Routing: <size> / <phase> / <subject> -> <category> (<lightweight|standard> mode, <skipping planning + quality | full pipeline>)
   ```
   Then return exactly one of:
   - `route: <plugin>:<skill>` plus the canonical spawn prompt for the matched route.
   - `block: <plugin> not installed` (or MCP variant) plus the exact install/auth instruction from the hard-block contract.
   - `clarify: <one disambiguation question>` when no row matches confidently. Ask exactly ONE question, then re-run the algorithm with the answer.

## Output contract

The skill returns text that team-lead consumes verbatim. The output is one of three shapes:

```
route: <plugin>:<skill>
spawn-prompt: <multi-line spawn prompt for the teammate or skill invocation>
```

```
block: <reason>
install: <exact command or auth instruction>
```

```
clarify: <one short disambiguation question>
```

team-lead does not paraphrase or expand the output. If `route` is returned, team-lead spawns the teammate (or invokes the skill) using the spawn-prompt as-is. If `block` is returned, team-lead emits the message verbatim and stops. If `clarify` is returned, team-lead asks the question and waits.

## Authoring rules for routing data

Contributors extend routing by editing `routing-table.md`, NOT by editing team-lead's prose or this SKILL.md.

- New categories use kebab-case names; tests assert against them.
- Every new row MUST be paired with a triggering test prompt in `tests/triggering/prompts/`.

## Related files

- [decision-tree.md](decision-tree.md) — classifier prose; three axes plus lightweight-mode heuristics.
- [routing-table.md](routing-table.md) — lookup table for step 3.
- [references/hard-block-contract.md](references/hard-block-contract.md) — hard-block message templates.

## Example routes

| User message | Expected routing output |
|---|---|
| "build user-auth" | `route: planner` (coding-feature-dev) |
| "why is the login page failing" | `route: superpowers:systematic-debugging` or `cpp-expert`/`ceditor-expert` by path (coding-debugging) |
| "write a design doc for the new billing flow" | `route: planner` (docs-authoring) |
