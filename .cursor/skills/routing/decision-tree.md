# Decision tree

Classify every ad-hoc request on three axes. The combination maps to a `category` in `routing-table.md`.

## Size

| Size | Signals |
|---|---|
| **trivial** | Single file, typo, rename, log line, one-liner, "quick fix" with named file |
| **small** | Single bug repro, one test failing, single PR comment, localized change |
| **medium** | "build", "implement", "add feature", multi-file scope, new module |
| **large** | Multi-day feature, cross-service, regulated/team-coordinated, explicit epic |

When ambiguous between trivial and medium, prefer `clarify` over guessing.

## Phase

| Phase | Signals |
|---|---|
| **explore** | "how does X work", "where is Y", "explain this code" |
| **design** | "design", "architecture", "blueprint", "plan the approach" |
| **implement** | "build", "implement", "add", "create", "wire up" |
| **test** | "add tests", "coverage", "TDD", "make tests pass" |
| **review** | "review", "audit", "look at this PR", "code review" |
| **debug** | "why is", "failing", "broken", "error", "bug", "doesn't work" |
| **ship** | "ship", "merge", "push", "create PR", "prepare release" |
| **oncall** | "incident", "outage", "production down", "pager" |
| **docs** | "write doc", "design doc", "PRD", "RFC", "documentation" |

## Subject

| Subject | Signals |
|---|---|
| **code** | Source files, functions, classes, tests (default when unclear) |
| **spec** | `.ai/specs/`, requirements, tasks.md, acceptance criteria |
| **config** | YAML, env, CI, build files, settings |
| **infra** | Deploy, k8s, terraform, cloud resources |
| **incident** | Production failure, alerts, on-call response |
| **PR** | Pull request, review comments, merge conflicts |

## Lightweight mode

After classifying size, set mode for team-lead's routing callout:

- **lightweight**: `trivial`, or `small` with single-file scope → "skipping planning + quality"
- **standard**: `medium` or `large` → "full pipeline"

Lightweight mode does **not** bypass `requires_plugin` / `requires_mcp` hard-blocks.

## Category mapping (quick reference)

| size | phase | subject | category |
|---|---|---|---|
| medium/large | implement | code | coding-feature-dev |
| small/trivial | implement | code | coding-small-fix |
| any | debug | code | coding-debugging |
| any | explore | code | coding-explore |
| any | design | code | coding-architecture |
| any | review | code/PR | coding-review |
| small | design | code | plan-driven-throwaway |
| any | docs | spec/code | docs-authoring |
| any | ship | PR/code | pr-ship |

When two categories match, prefer the more specific row in `routing-table.md`.

## Disambiguation triggers

Ask one `clarify` question when:

- Persistent spec (`.ai/specs/<feature>/`) vs throwaway plan
- Small fix vs full feature pipeline
- Code review vs security audit vs refactor
