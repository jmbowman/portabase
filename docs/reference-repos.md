# Reference repos

External repositories cloned locally for reference in this project. Mechanics (how clones are managed, added, removed, refreshed) are documented in the `reference-repos` Claude Code skill — this file is just the project-specific list: what's here and why.

## Current repos

### apm

Path: `.reference-repos/apm/`

Pinned to `v0.24.0` — the exact version `mise.toml` installs (`apm = "latest"` currently resolves here; re-pin if `mise.lock`'s `tools.apm.version` moves). [Microsoft's Agent Package Manager](https://microsoft.github.io/apm/), used to install the LID plugins (see `apm.yml`). Check here first for `apm`'s CLI behavior, `apm.lock.yaml` schema, or deployment/gitignore conventions — faster and more exact than `--help` or the public docs site, which is generated from this same repo's `docs/` tree anyway.

### lid

Path: `.reference-repos/lid/`

Pinned to `v1.3.0` — the exact tag `apm.yml` deploys `linked-intent-dev` and `arrow-maintenance` from. Check here first when a deployed LID skill under `.claude/skills/` behaves unexpectedly, or its rendered docs are unclear — also includes undeployed parts of the plugin repo (tests, design notes) that don't ship with the deployed skill.

### mdbook

Path: `.reference-repos/mdbook/`

Pinned to `v0.5.4` — the exact version `mise.toml` installs (`mdbook = "latest"` currently resolves here; re-pin if `mise.lock`'s `tools.mdbook.version` moves). Renders `docs/` to HTML via `mise run mdbook-build`, local browsing only for now. Check here first for `book.toml`/`SUMMARY.md` syntax or build-behavior questions.

### mdbook-lint

Path: `.reference-repos/mdbook-lint/`

Pinned to `v0.14.4` — the exact version `mise.toml` installs (re-pin if `mise.lock`'s `tools."github:joshrotenberg/mdbook-lint".version` moves). Lints `docs/`'s markdown as an `mdbook` preprocessor. Check here first for rule behavior or `.mdbook-lint.toml` config questions.

### mise

Path: `.reference-repos/mise/`

Pinned to `v2026.7.5`. The tool-version manager `mise.toml`/`mise.lock` are written for. Check here first for backend behavior (e.g. `ubi` vs `github:`, `aqua:`) or `mise.toml` syntax questions — this is how the `ubi` backend's deprecation (in favor of `github:`, removal slated for 2027.1.0) was confirmed, faster than the public docs site which mirrors this same repo's `docs/` tree.

### cursor-plugins

Path: `.reference-repos/cursor-plugins/`

Pinned to the tip of `main` as of this checkout — Cursor's official plugin monorepo has no version tags for individual plugins. Cloned to evaluate `thermos/` (`skills/thermos`, `skills/thermo-nuclear-review`, `skills/thermo-nuclear-code-quality-review`), a dual-subagent code-review skill, as a candidate for this repo's code-review harness. Despite the Cursor-only install path (`.cursor-plugin/plugin.json`), the skill content itself is plain Agent Skills `SKILL.md` + Claude Code `Task`-tool subagent invocations (`subagent_type`/`run_in_background`), so it's portable here. Check here first when evaluating or adapting thermos, rather than re-fetching from GitHub.

### qt-agent-skills

Path: `.reference-repos/qt-agent-skills/`

Pinned to the tip of `main` as of this checkout — no version tags. The Qt Company's official Agent Skills collection (BSD-3-Clause code, but commercial use requires accepting separate Qt AI Services Terms & Conditions — check `LICENSE`/`README.md` in the clone before adopting anything from here for real use). Cloned to evaluate `skills/qt-cpp-review/` as a candidate for this repo's code-review harness: a deterministic lint pass (60+ rules) plus six parallel Qt/C++-domain review agents (ownership/lifecycle, thread safety, model contracts, etc.). Tuned for Qt 6 — per Qt's own announcement, "Qt5 projects will mostly work but some checks are Qt6-specific," relevant since PortaBase is still on Qt 5.15/QWidgets. Check here first when evaluating qt-cpp-review or any of the repo's other Qt-focused skills.

### qtbase

Path: `.reference-repos/qtbase/`

Pinned to `v5.15.2` — the exact Qt version CI installs via `install-qt-action`. Covers QtCore, QtGui, QtWidgets, QtXml, QtPrintSupport (everything `portabase.pro` pulls in via `QT +=` on the currently-supported platforms). Check here first when a Qt Widgets/printsupport/XML API behaves unexpectedly — pinned to the exact version CI builds against, so more reliable than the installed Qt docs for telling a PortaBase bug from a Qt quirk.

### qtsvg

Path: `.reference-repos/qtsvg/`

Pinned to `v5.15.2`. Covers the `svg` module, used only in the Android build (`QT += ... svg` in the `android-clang` block of `portabase.pro`). Check here first for Android-specific SVG rendering questions.

### qtandroidextras

Path: `.reference-repos/qtandroidextras/`

Pinned to `v5.15.2`. Covers the `androidextras` module (`QT += androidextras` in the `android-clang` block). Check here first for Android JNI/platform-integration questions.

### qtmacextras

Path: `.reference-repos/qtmacextras/`

Pinned to `v5.15.2`. Covers the `macextras` module (`QT += macextras` in the `macx` block). macOS builds are currently disabled in CI (see `.github/workflows/build.yml`), but the code path is still live in `portabase.pro`/`packaging/mac/`. Check here first for macOS-specific integration questions.

### compiledb

Path: `.reference-repos/compiledb/`

Pinned to `0.10.7` — the exact version `packaging/lint_cpp.py`'s `uv run --script` header installs (see its `dependencies = ["compiledb"]` block; re-pin if that version pin changes). Generates `compile_commands.json` from a `make -n` dry run for `clang-tidy`/`clazy`. Check here first for its update/merge-mode semantics (`compiledb/__init__.py`'s `merge_compdb()`), CLI flags, or parser behavior (`compiledb/parser.py`) before trusting `--help` or guessing — this is how a bug in `lint_cpp.py`'s two-shadow-build compile-db generation was confirmed: `merge_compdb()` keys entries by `os.path.join(directory, file)`, which collapses to just `file` whenever paths are absolute (as they are here via qmake's `$$PWD`), so a second pass silently overwrites the first pass's entries for every file compiled in both instead of merging them.

<!--
Per-repo entries go here as they're added, e.g.:

### metakit-upstream

Path: `.reference-repos/metakit-upstream/`

Consult when verifying how upstream metakit implements a feature before patching the vendored copy in `metakit/`.
-->
