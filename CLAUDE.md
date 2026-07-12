# PortaBase

Portable tabular database application, C++ / Qt (currently Qt 5.15, QWidgets), ~25 years old, historically a solo project. Currently being revived — see `docs/wiki/roadmap.md` for the active plan and `docs/wiki/agentic_sdlc.md` for the philosophy behind the agentic tooling in this repo.

## Where to look first

- **`docs/wiki/`** — shared project knowledge, plain markdown, edited with native file tools.
  - `docs/wiki/agentic-development-support.md` — how the agentic tooling in this repo fits together.
  - `docs/wiki/roadmap.md` — the active development plan and its rationale.
  - `docs/wiki/agentic_sdlc.md` — the maintainer's wishlist for how AI agents should work in this codebase.
  - `docs/wiki/competitive-landscape.md` — survey of nearby tools and why none of them replace PortaBase.

Check these before doing independent research on project history or direction — the maintainer has often already thought through and documented the "why."

## Build system

qmake-based, not CMake (except for the vendored `metakit/` library). Typical local build:

```
qmake portabase.pro
make
```

Platform-specific build/packaging scripts live under `packaging/{linux,mac,windows,android}/`. CI builds all platforms via `.github/workflows/build.yml` (Linux always; macOS is currently disabled — no viable Qt 5 runner remains, see the comment block in that file; Windows and Android build normally).

## Directory map

- `src/` — main application source (~485 `.cpp` + ~235 `.h` files). Subdirectories: `calc/` (calculated-field expression engine + editor UI), `color_picker/`, `encryption/`, `image/`, `qqutil/` (Qt utility helpers).
- `metakit/` — vendored, patched embedded database library (the current storage backend; see `docs/wiki/roadmap.md` for the planned migration to Turso).
- `resources/` — help documentation (Sphinx) and Qt Linguist translations.
- `packaging/` — per-platform build/installer scripts, plus `update_help_translation_templates.py` (regenerates help-doc translation `.pot` templates; wired up as a `prek` pre-commit hook).
- `doc/`, `docs/` — `doc/` is legacy Doxygen output config; `docs/` is the new agentic-tooling documentation tree (`docs/wiki/`, and `docs/intent/` for Linked-Intent Development specs).
- `debian/` — Debian packaging control files.
- `site/` — new project website source (in progress, see `docs/wiki/roadmap.md`).

## Dev tooling

Tool versions are pinned via `mise` (`mise.toml`/`mise.lock`) — run `mise install` to get everything. Currently manages: `actionlint` (lints `.github/workflows/`), `prek` (git hooks, see `prek.toml`), `apm`, `pipx:vcs2l` (reference-repo clone management, see `docs/reference-repos.md`), and `pipx:gh2md` (GitHub issues/PRs mirror, see below). Useful tasks: `mise run lint-workflows`, `mise run pre-commit`, `mise run install-hooks`.

`prek` hooks (defined in `prek.toml`) currently just regenerate help translation templates when `resources/help/` changes; enforced both locally (`mise run install-hooks`) and in CI (`pre-commit` job in `build.yml`).

**`.issues/`** is a gitignored, offline mirror of this repo's GitHub issues/PRs (including comments) for agent context, refreshed via `mise run issues-sync`. It uses `gh2md`, authenticated via a `GITHUB_ACCESS_TOKEN` env var or a token file (`~/.config/gh2md/token`/`~/.github-token`) that the maintainer manages outside of any Claude Code session. **Do not read, cat, echo, or otherwise access that token, its env var, or its file from an agent session** — refreshing `.issues/` is the maintainer's manual step.

Claude Code skills/commands/MCP dependencies sourced from third-party repos (like the LID plugins below) are declared in `apm.yml`/`apm.lock.yaml` and installed via `mise exec -- apm install` — see `docs/wiki/agentic-development-support.md` for details, including the gotcha where plain `owner/repo` silently no-ops for monorepo plugins (needs `owner/repo/path/to/plugin#ref`).

## Testing

**There is no automated test suite yet.** Adding one is a planned roadmap step, deliberately sequenced after this agentic-scaffolding work and before further feature work. Until then, verify changes by building and exercising the app manually — don't assume test coverage exists for any given code path.

## Known issues

There's a known crash bug applying schema changes to an existing database from the dbeditor dialog (`src/dbeditor.cpp`) — see `docs/wiki/roadmap.md`. Treat schema-change code paths in `dbeditor.cpp`/`database.cpp` with extra care.

## LID
- Mode: Scoped
- Version: 1.3.0

## LID Scope

Piloted on the column-editing subsystem only; not yet adopted project-wide.

Paths in scope:
- `src/dbeditor.cpp`
- `src/dbeditor.h`
- `src/database.cpp` (schema-mutation methods only: `setIndex`, `setDefault`, `addColumn`, `deleteColumn`, `renameColumn`, `updateDataFormat`, `addViewColumn` — not the whole 2696-line file)
- `src/database.h` (declarations for the above)

Paths explicitly excluded:
- everything else in `src/database.cpp`/`.h` outside the schema-mutation methods listed above

## Linked-Intent Development (MANDATORY, within scope above)

**Consult the `linked-intent-dev` skill for ALL code changes within the scope above.** All changes flow through the arrow of intent in one direction:

```
HLD → LLDs → EARS → Tests → Code
```

- **New features and refactors**: full six-phase workflow (HLD check → LLD check/draft → EARS → intent-narrowing edge audit → tests-first → code).
- **Bug fixes**: walk the arrow like any other change — find where behavior diverged from intent and cascade from there. No short-circuit.
- **If unsure**: use the full workflow.

Stop after each phase for user review. **Docs carry current intent, written to be read cold** — write each doc as if authored fresh today, from current intent alone: no narration of how it changed, no meaning that needs the conversation that produced it, no rebuttals to questions only a past discussion raised. Rationale, considered alternatives, and constraints a fresh author would independently write stay; record rejected alternatives and why in the LLD's Decisions & Alternatives table, not as asides in body prose.

**Memory vs. intent.** Before saving durable project knowledge to agent or tool memory, test whether it is project *intent* — would a fresh agent, in any tool, next session, need it to build this system correctly? If yes, record it in the arrow (HLD / LLD / EARS / decision doc), which travels and cascades — not in private, per-tool memory, where intent escapes the arrow. Knowledge about the user or how they like to work stays in memory. **Open question for this project**: general project knowledge that isn't code-behavior-specific (e.g. `docs/wiki/`'s pages) doesn't obviously belong in the arrow either — the boundary between "the arrow" and `docs/wiki/` hasn't been resolved yet; use judgment and flag ambiguous cases to the user rather than silently picking one.

### Navigation

| What you need | Where to look |
|---|---|
| High-level design | `docs/high-level-design.md` |
| Design tree (sub-HLDs, LLDs, their specs) | `docs/intent/` — one folder per node |
| EARS specs | beside each design doc as `{node}-specs.md` in the node's folder under `docs/intent/` |
| Decision docs | `docs/decisions/` (project-level) and `docs/intent/<segment>/decisions/` |
| Arrow of intent overlay | `docs/arrows/index.yaml` and per-segment docs in `docs/arrows/` |

### Terminology

- **HLD**: High-Level Design — single project-level doc at `docs/high-level-design.md`.
- **LLD**: Low-Level Design — detailed component design doc in `docs/intent/`. The design layer is a recursive tree: the root is the HLD, leaf LLDs own EARS, and a component deep enough to outgrow one doc becomes a sub-HLD (HLD-shaped, owns no EARS) with children beneath it. "HLD" and "LLD" are roles by position; depth-2 (one HLD over flat leaf LLDs) is the default.
- **EARS**: Easy Approach to Requirements Syntax — structured one-line requirements beside each design doc as `{node}-specs.md` in the node's folder under `docs/intent/`. IDs are path-concatenated — the root-to-leaf path of the owning segment plus a number — so a prefix grep gathers a subtree. Markers: `[x]` implemented, `[ ]` active gap, `[D]` deferred.
- **Arrow**: the unidirectional chain from vision to code (HLD → LLDs → EARS → Tests → Code). Strictly a DAG of intent.
- **Arrow segment**: the territory owned by one leaf LLD — the LLD itself plus the specs, tests, and code that cite its EARS IDs. The boundary is the leaf prefix. Within-segment cascade is free; across-segment cascade pauses.
- **Cascade**: propagating a change downstream through the arrow so adjacent levels stay coherent.

### Code annotations

Annotate code and tests with `@spec` comments citing EARS IDs:

```
// @spec COL-UI-001, COL-UI-002
```

Place the annotation at the *entry point of the behavior's implementation graph* — the topmost function or module owning the specified behavior, not every helper. When a behavior spans multiple subsystems (UI + API + database, for example), annotate at the entry point in each subsystem. Tests follow the same rule: annotate the test that directly exercises the spec, not every inner assertion.

**Not yet applied to `dbeditor.cpp`/`database.cpp` in this pilot** — see `docs/wiki/lid-feedback.md` issue #1 for why (ambiguity in LID's own docs about whether brownfield mapping is meant to add these automatically). Add them deliberately, on purpose, the next time this code is touched — don't assume they're already there.
