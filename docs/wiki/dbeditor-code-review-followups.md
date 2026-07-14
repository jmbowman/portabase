# Column-Editing Code Review Followups

Findings from a `qt-cpp-review` deep audit of `src/dbeditor.{cpp,h}`, `src/database.{cpp,h}`, and `src/columneditor.{cpp,h}` — run while investigating the `COL-DB-007` add-column crash (see `docs/intent/column-editing/column-editing-design.md` § Open Questions & Future Decisions). None of these are the crash itself; they're real but lower-priority items surfaced along the way, deferred here rather than bundled into that investigation.

## Unguarded `c4_View::Find()` Results

About ten call sites across `database.cpp` and `dbeditor.cpp` do `int i = view.Find(...)` and immediately index `view[i]` with no check for `i == -1`:

- `database.cpp`: `setIndex` (~555), `setDefault` (~594), `renameColumn` (~885)
- `dbeditor.cpp`: `editColumn` (~188), `deleteColumn` (~305), `moveUp`/`moveDown` (~339, 342, 362, 366), `renameColumnRefs` (~629), `deleteColumnRefs` (~649)

Under the current, intended call sequences these all look safe today — nothing yet drives them with a name that isn't already known to exist. But nothing defends against it either: a `Find()` miss silently indexes the view at `-1`, which the vendored Metakit doesn't fail fast on (confirmed by reading `metakit/include/mk4.inl` and `metakit/src/column.cpp`/`viewx.cpp`) — it builds a cursor that only crashes *lazily*, the first time a property on that row is read or written, with no assert in release builds. A loaded footgun rather than an active bug, but worth guarding defensively (matching the one correct existing instance in `DBEditor::applyChanges()`, `dbeditor.cpp:513`, which does check).

## Other Investigation Targets

- **`Database::addColumn()`'s ID-gap-scan has no defense against a pre-existing duplicate `cId`** (`database.cpp:698-717`). The "find the next available ID" loop assumes existing `cId` values are unique; it has no assertion or check for that precondition. A duplicate (from any future corruption) would desync the scan silently.
- **`Database::commit()` discards Metakit's `bool` success/failure return** (`database.cpp:2393-2400`). `storage->Commit()`/`file->Commit()` both signal real I/O failures (disk full, write error) via their return value; `commit()` ignores both, so a failed save currently looks identical to a successful one to the caller.
- **`DBEditor::edit()` never resets staged state** (`dbeditor.cpp:99-125`). It repopulates `info`/`calcMap`/`decimalsMap`/`deletedCols` without clearing them first, so calling `edit()` twice on the same `DBEditor` instance would leak `CalcNode*` values and duplicate rows in `info`. Not reachable today — the only call site (`portabase.cpp:383`) constructs a fresh `DBEditor` per invocation — but worth hardening if the dialog is ever pooled/reused.

## Lint Findings (Style/Modernization)

`mise run lint-cpp` (cppcheck, via `prek.toml`'s `lint-cpp` hook) now runs automatically against new/changed C++ files on every commit — but that only covers files touched going forward, not retroactively. Running the full `qt_review_lint.py` deterministic linter (from the `qt-cpp-review` skill) against the reviewed files surfaced ~35 pre-existing findings, none crash-related, all cheap fixes whenever these files are next touched substantively:

- **`DEP-10` (.count()/.length() → .size())** — ~20 call sites, mostly in `database.cpp`, a few in `dbeditor.cpp`. Pure style; `.count()`/`.length()` and `.size()` are equivalent on Qt containers.
- **`API-5` (get-prefix on plain getters)** — ~11 call sites, mostly in `database.h` (`getIndex`, `getType`, `getDefault`, etc.), one in `dbeditor.h`. Qt convention reserves the `get` prefix for user-interaction/decomposition getters; a rename would be a real API-shape change across every caller, so lower priority than the others here.
- **`PAT-7` (`QMap` usage)** — 2 sites in `dbeditor.h` (`NameCalcMap`, `NameDecimalsMap` typedefs). Flagged as "verify copying is needed; `std::map` saves ~1.7KiB text" — worth a look, not urgent.
- **`ENM-2` (unscoped enum without explicit underlying type)** — 1 site, `database.h:45` (`Database::OpenResult`). Adding an explicit underlying type would help binary compatibility if this enum's values are ever persisted/exchanged externally.
- **`DEP-13` (`QChar` as object type)** — 1 site, `columneditor.cpp:242`.

Regenerate the full file:line list any time with:

```bash
python3 .claude/skills/qt-cpp-review/references/lint-scripts/qt_review_lint.py src/dbeditor.cpp src/dbeditor.h src/database.cpp src/database.h src/columneditor.cpp src/columneditor.h
```

## Build/Test Infrastructure Followups

- **`lint_cpp.py` doesn't run clang-tidy/clazy.** They need a compile database, and the test suite currently recompiles the same shared sources as the app with different flags — revisit alongside splitting the shared sources into a library target both link against (see `docs/wiki/compiledb-bugs.md` for a compiledb defect worth re-checking then).
- **`mk4.h`/`mk4.inl`/`mk4str.h`/`mk4str.inl` are manually copied into `build/` and `tests/dbeditor/build/`, not make-tracked.** `qmake portabase.pro && make` needs these present in the build directory (the `-Ibuild` include path resolves them) but nothing in the generated Makefiles actually places them there — `debian/rules` copies them as an explicit packaging step (`cp metakit/include/mk4.h metakit/include/mk4.inl metakit/include/mk4str.h metakit/include/mk4str.inl build/`), which the plain `qmake && make` flow documented in `CLAUDE.md`/`doc/install_linux.txt` never runs. Because the copy isn't a make-generated artifact, neither `make clean` nor `make distclean` removes it — a full reset of either build directory needs a manual `rm -rf`. Worth either making this copy step part of the qmake project itself (a `QMAKE_POST_LINK`-style pre-build copy, or a documented one-time setup step) so it's both automatic and properly clean-tracked.
