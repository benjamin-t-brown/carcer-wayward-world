# Hard-block contract

Emit these messages verbatim when routing dep-checks fail. Always end with "Then re-run your request."

## Plugin missing

```
This route requires the **{plugin}** plugin, which is not installed (or status is unknown).

Install it, then re-run your request.
```

Substitute `{plugin}` with the value from `requires_plugin` (e.g. `superpowers`).

## MCP auth needed

```
This route requires the **{mcp}** MCP server, which needs authentication.

Authenticate or install the MCP, then re-run your request.
```

## MCP missing

```
This route requires the **{mcp}** MCP server, which is not installed.

Install and configure the MCP, then re-run your request.
```

## Multiple deps missing

```
This route requires dependencies that are not available:

{bullet list of each missing plugin or MCP}

Resolve the items above, then re-run your request.
```

## Delegation unavailable (Cursor nested subagent)

```
Parallel subagent delegation must be available for the harness to function. Start a new Cursor Agent session with team-lead as the primary agent (not as a nested @-mention), then re-run your request.
```
