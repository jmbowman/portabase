# PortaBase

Portable tabular database application, C++ / Qt (currently Qt 5.15, QWidgets), ~25 years old, historically a solo project. Currently being revived — see `docs/wiki/roadmap.md` for the active plan and `docs/wiki/agentic_sdlc.md` for the philosophy behind the agentic tooling in this repo.

## Where to look first

- **`docs/wiki/`** — shared project knowledge, plain markdown, edited with native file tools.
  - `docs/wiki/agentic-development-support.md` — how the agentic tooling in this repo fits together.
  - `docs/wiki/roadmap.md` — the active development plan and its rationale.
  - `docs/wiki/agentic_sdlc.md` — the maintainer's wishlist for how AI agents should work in this codebase.
  - `docs/wiki/competitive-landscape.md` — survey of nearby tools and why none of them replace PortaBase.

Check these before doing independent research on project history or direction — the maintainer has often already thought through and documented the "why."

## Comments & explanations

Keep comments in code/config brief — a pointer, not the explanation. When something needs real justification (why this tool version, why this workaround, why this tradeoff), put it in `docs/wiki/` (or a Decision doc under `docs/decisions/` for a contested choice worth recording alternatives for) and reference it by path from a one-line comment. See `docs/wiki/cppcheck-bugs.md` plus its one-line pointer in `.github/workflows/build.yml` for the pattern.

When documenting this repo's actual state or conventions, say it directly — don't wrap a decision that has already been made in generic hedging ("this can live wherever the project's conventions dictate", "check before assuming"). Reserve portable phrasing for content genuinely meant to travel unchanged to other repos, such as a skill's general mechanism section.

## Making changes

The bottleneck in this project is the maintainer's review budget, not how fast code gets written. Optimize diffs for being understood.

- **Keep PRs small** — under ~400 lines added or changed where the work divides that way. Don't bundle adjacent value (a new feature, a test-suite expansion) into a bug-fix PR unless that volume was genuinely needed to *find* the bug; when it was, say so.
- **Match surrounding idioms in files you edit**, even over more modern equivalents. Idiom-consistent, boring diffs clear review fast; novel-but-equivalent patterns force line-by-line scrutiny. An anonymous namespace where the rest of the file uses none, or a fresh loop variable where the method reuses `i`, is the kind of thing that stands out to a human reader and that an agent doesn't think to flag. Reserve modern C++ for brand-new files, where there is no surrounding style to match and so no inconsistency is created.
- **For an unavoidably large diff**, offer a reviewer's guide — a risk-ordered reading order and a consolidated list of decisions and trade-offs — rather than leaving the reviewer to reconstruct intent from the code. Surface your own low-confidence areas without being asked.

## Copyright headers

Every `.cpp`/`.h` file — application **and** tests — opens with the project's copyright + license header: a leading `/* */` block naming the file, then `(c) <years> by Jeremy Bowman <jmbowman@alum.mit.edu>`, then the GPL-v2-or-later summary (copy the block verbatim from any existing `src/` file). The years are the years the file was actually touched: a **new** file gets the current year; an **edited** file gains the current year appended to its list, collapsing consecutive years into ranges (e.g. `2015-2017,2026`). Maintain this on every create/update — it is easy to forget, especially on test files, which need the header just as much as `src/`. Application source follows the license block with a `/** @file … */` doxygen block; tests need only the license block (no doxygen, and don't repeat the filename in the description comment below it).

## Docstrings

Document with Doxygen `/** … */` blocks, following the existing layout:
- **Methods**: a full block (with `@param`/`@return`) on the *definition* in the `.cpp`, for every method including trivial accessors — **never** on the `.h` declaration.
- **Headers (`.h`)**: type-level docs only — a `/** … */` on each class/struct/enum, and a trailing `/**< … */` on each enum value and data member. The declarations themselves stay bare.
- **`@spec` annotations** (LID) go at the implementation entry point — the `.cpp` definition — not the `.h` declaration.

Test code follows the same rules — including the `.h`-bare/`.cpp`-documented split for the free-function helpers in `tests/common/` — plus one addition: every `QTest` private slot gets a short doc block describing the scenario it sets up and what it asserts, except `initTestCase()` and the `_data()` half of a table-driven pair (document the pair once, on whichever of the two reads better).

## Changelog (`CHANGES`)

The maintainer curates `CHANGES` by hand — don't add or edit entries unless asked. When you are asked, match the existing format: a header line `YYYY-MM-DD  INITIALS  short summary` (two spaces between each field; `INITIALS` is the author's, e.g. `JMB`), a blank line, then a 4-space-indented body — either `* ` bullets or a short prose paragraph — wrapped at ~72 columns. Keep each item a terse, past-tense line; lead with changes that matter to end users, and include only the *major* changes relevant to other developers. No explanations of deferred or future work.

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

Tool versions are pinned via `mise` (`mise.toml`/`mise.lock`) — run `mise install` to get everything. Currently manages: `actionlint` (lints `.github/workflows/`), `prek` (git hooks, see `prek.toml`), `apm`, `pipx:vcs2l` (reference-repo clone management, see `docs/reference-repos.md`), `pipx:gh2md` (GitHub issues/PRs mirror, see below), and `mdbook`/`github:joshrotenberg/mdbook-lint` (renders and lints `docs/`, see `docs/wiki/agentic-development-support.md`). Useful tasks: `mise run lint-workflows`, `mise run pre-commit`, `mise run install-hooks`, `mise run mdbook-build`, `mise run mdbook-lint`.

`prek` hooks (defined in `prek.toml`) regenerate help translation templates when `resources/help/` changes, and lint `docs/` (excluding the LID design tree) when relevant doc files change; enforced both locally (`mise run install-hooks`) and in CI (`pre-commit` job in `build.yml`).

Before running a hook, `prek` stashes unstaged working-tree changes to its own patch cache at `~/.cache/prek/patches/` — **not** a git stash, so `git stash list` shows nothing and the changes look lost. Killing a `prek run` mid-execution can leave them unrestored; recover by `git apply`-ing the most recent patch file there. Worth knowing before reflexively killing a slow-looking prek invocation.

Python tooling scripts (e.g. `packaging/lint_cpp.py`) use `uv` with PEP 723 inline script dependencies, not manual `pip install`. `uv` itself is deliberately *not* mise-managed — it's expected to already be globally available on a dev machine; CI provisions it via the `astral-sh/setup-uv` action in `build.yml`.

**`.issues/`** is a gitignored, offline mirror of this repo's GitHub issues/PRs (including comments) for agent context, refreshed via `mise run issues-sync`. It uses `gh2md`, authenticated via a `GITHUB_ACCESS_TOKEN` env var or a token file (`~/.config/gh2md/token`/`~/.github-token`) that the maintainer manages outside of any Claude Code session. **Do not read, cat, echo, or otherwise access that token, its env var, or its file from an agent session** — refreshing `.issues/` is the maintainer's manual step.

Claude Code skills/commands/MCP dependencies sourced from third-party repos (like the LID plugins below) are declared in `apm.yml`/`apm.lock.yaml` and installed via `mise exec -- apm install` — see `docs/wiki/agentic-development-support.md` for details, including the gotcha where plain `owner/repo` silently no-ops for monorepo plugins (needs `owner/repo/path/to/plugin#ref`).

## Testing

A QtTest suite has started, currently covering the Columns Editor dialog (`tests/dbeditor/`) — build and run it with `qmake portabase-tests.pro && make`. It isn't wired into CI yet, and coverage is still narrow: don't assume test coverage exists for any given code path, and verify changes by building and exercising the app manually where it doesn't.

## Known issues

There's a known crash bug applying schema changes to an existing database from the dbeditor dialog (`src/dbeditor.cpp`) — see `docs/wiki/roadmap.md`. Treat schema-change code paths in `dbeditor.cpp`/`database.cpp` with extra care.

## LID
- Mode: Scoped
- Version: 1.3.0

## LID Scope

Piloted on the Edit Columns operation (end-to-end, as the user experiences it), the data-integrity-check feature, and the data-row lifecycle (adding, copying, deleting, and row-ID assignment); not yet adopted project-wide.

Paths in scope:
- `src/dbeditor.cpp`, `src/dbeditor.h`
- `src/columneditor.cpp`, `src/columneditor.h`
- `src/portabase.cpp` (`PortaBase::editColumns` only — not the rest of the file)
- `src/database.cpp` (schema-mutation methods: `setIndex`, `setDefault`, `addColumn`, `deleteColumn`, `renameColumn`, `updateDataFormat`; the view-maintenance methods the Edit Columns operation drives: `addViewColumn`, `deleteViewColumn`, `setViewColumnSequence`, `reconcileAllView`, `getView`, `listColumns`, `formatString`; and the calculated-column recalculation the operation triggers on accept: `calculateAll`, `getRow` — not the whole 2696-line file)
- `src/database.h` (declarations for the above)
- `src/integritycheck.cpp`, `src/integritycheck.h` (data-integrity checker core)
- `src/roweditor.cpp`, `src/roweditor.h` (`RowEditor::edit` when it creates a row — the add and copy branches; not in-place editing of an existing row)
- `src/datamodel.cpp`, `src/datamodel.h` (`addRow`, `deleteRow`, `deleteAllRows` only)
- `src/viewdisplay.cpp`, `src/viewdisplay.h` (`addRow`, `editRow`, `deleteRow`, `deleteAllRows` only)
- `src/view.cpp` (`View::deleteAllRows` only — not the rest of the file)
- `src/portabase.cpp` (`PortaBase::deleteRow`, `copyRow`, `deleteAllRows` — the Row menu actions, alongside `editColumns`)
- `src/database.cpp` (row-lifecycle methods: `addRow`, `deleteRow`, `compressRowIds`, and the `maxId` initialization in `load` — not the rest of `load`)
- `src/commandline.cpp`, `src/commandline.h` (the `check` subcommand only — not the rest of the CLI)
- `src/main.cpp` (the CLI-dispatch condition only — routing `check` to the command-line interface)
- `tests/dbeditor/`, `tests/dbeditor_robustness/`, `tests/integritycheck/`

Paths explicitly excluded:
- everything else in `src/database.cpp`/`.h` outside the methods listed above
- everything else in `src/commandline.cpp` outside the `check` subcommand
- everything else in `src/portabase.cpp` outside `PortaBase::editColumns` and the Row menu actions listed above
- everything else in `src/view.cpp` outside `View::deleteAllRows`
- `src/csvutils.cpp`, `src/importutils.cpp` — the CSV/XML import paths drive `Database::addRow` but are a separate operation, outside the row-lifecycle segment

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
