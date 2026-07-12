# Agentic Development Support

PortaBase is being revived with heavy use of Claude Code and other AI coding agents (see [roadmap.md](roadmap.md) for the active plan and [agentic_sdlc.md](agentic_sdlc.md) for the motivation and wishlist behind this). This page documents what's actually set up, so both human contributors and their agents know what exists and why.

## Shared Project Knowledge Base: `docs/wiki/`

`docs/wiki/` is a plain-markdown knowledge base — architecture rationale, decisions, "why" documentation, the active development plan, and the maintainer's agentic-tooling wishlist — edited directly with native file tools (`Read`/`Write`/`Edit`/`Grep`/`Glob`), no special server or sync layer involved. `docs/arrows/`, `docs/intent/`, `docs/high-level-design.md`, and `CLAUDE.md` (repo root) are separate documentation trees with their own conventions (see the LID section below); `docs/wiki/` is for everything else.

[roadmap.md](roadmap.md), [agentic_sdlc.md](agentic_sdlc.md), and [competitive-landscape.md](competitive-landscape.md) were folded in here from the repo root. Rendering `docs/wiki/` through a static-site generator (mdBook is the current front-runner, chosen for build speed and low maintenance overhead) is still an open, undecided future step. Whatever `docs/wiki/` content gets written should stay plain-Markdown-portable (no tool-specific link syntax, no required frontmatter schema) so that move stays easy.

## What Lives in Native Claude Code Memory Instead

Claude Code also has a private, per-developer memory system (not part of this repo — it lives under the developer's own `~/.claude/` directory). The split is:

- **Personal working-style notes** (how a specific developer likes to collaborate) stay in that private memory.
- **Project knowledge** (architecture, rationale, decisions like why no competitor has emerged — see [competitive-landscape.md](competitive-landscape.md)) belongs here in `docs/wiki/` instead, since other contributors and their agents need to see it too.

## Agent Skill/Plugin Dependencies: APM (Agent Package Manager)

`.claude/skills/`, `.claude/commands/`, and MCP server dependencies sourced from third-party repos are declared in `apm.yml` and locked in `apm.lock.yaml` (Microsoft's [Agent Package Manager](https://microsoft.github.io/apm/), installed as a pinned `mise` tool). This replaces ad hoc `claude plugin install` — everything a contributor's agent needs is reproducible via `mise exec -- apm install`, the same "declare it in a manifest, get the same setup everywhere" pattern as `mise.toml` for CLI tools.

Notes for adding a dependency from a monorepo (like LID below): the shorthand `owner/repo` only works when the plugin/skill lives at the repo root. For a subdirectory, use `owner/repo/path/to/plugin#ref` (or the `git:`/`path:`/`ref:` object form in `apm.yml` directly) — plain `owner/repo` silently fetches the whole repo into `apm_modules/` without deploying anything if there's nothing deployable at the root.

## Linked-Intent Development (LID)

[`jszmajda/lid`](https://github.com/jszmajda/lid) — two-way-linked specs (why decisions were made, traced through HLD → LLD → EARS specs → tests → code). Installed via APM, pinned to `v1.3.0`:

```yaml
dependencies:
  apm:
    - jszmajda/lid/plugins/linked-intent-dev#v1.3.0
    - jszmajda/lid/plugins/arrow-maintenance#v1.3.0
```

`linked-intent-dev` is the greenfield HLD→LLD→EARS→tests→code workflow; `arrow-maintenance` (with its `map-codebase` command) is the brownfield tool for mapping _existing_ code into that structure bottom-up — the relevant one for PortaBase, since this is a 25-year-old codebase with no test suite yet. First real use: piloting `arrow-maintenance:map-codebase` against `src/dbeditor.cpp`/`.h` and the schema-change path in `src/database.cpp`, chosen because there's a known crash bug there worth surfacing. `lid-experimental` (a third plugin bundling not-yet-promoted techniques) was deliberately left out.

## Dynamic Workflows

Claude Code's built-in multi-agent orchestration (GA as of v2.1.154+), used ad hoc for loop-until-converged tasks like code review, rather than a custom orchestration framework. This is a per-developer Claude Code client setting, not repo config — check your own `/config` rather than assuming. As of this writing, the general auto-trigger is off (`Dynamic workflows: false`) but the explicit `ultracode` keyword still works regardless (`Ultracode keyword trigger: true`, size `unrestricted`, worktree base ref `fresh`) — the intended usage here is always the explicit keyword, never implicit/automatic triggering.

## Context Curation

A small `.claude/skills/context-curation-check/` skill (planned, not yet written) that checks this wiki, memory, and [roadmap.md](roadmap.md)/[agentic_sdlc.md](agentic_sdlc.md) before defaulting to independent research, and offers to search connected Gmail/Drive/Calendar for existing notes first.

## Reference Repos

External repos cloned locally for fast, exact reference answers (e.g. checking how LID actually behaves, or which Qt modules a given `QT +=` line needs) — see `docs/reference-repos.md` for the current list and the `reference-repos` Claude Code skill for the mechanics.

The `reference-repos` skill itself has no PortaBase-specific content and would be a reasonable candidate to extract into a standalone personal-tooling repo, consumed back into projects via APM the same way `jszmajda/lid`'s plugins are today. Deferred deliberately — it's a distraction from PortaBase's own roadmap while this pattern is still unproven. Revisit if it proves successful enough to want elsewhere.
