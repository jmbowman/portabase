# Arrow: row-lifecycle

The lifecycle of a data row — adding one (including copy-as-new), deleting one or many, and the `_id` assignment all of them depend on.

## Status

**AUDITED** — segment opened 2026-09-30 (git SHA `b94bbb9`); audited 2026-10-17. The LLD is drafted and reconciled with the code, and the EARS specs for addition, copy, row identity, and the `Database` side of deletion are drafted. Every ROW-DB spec except the newest, ROW-DB-012, has a test in `tests/rowlifecycle/`: the implemented ones pass and the active gaps fail as expected. ROW-DB-011's test reproduced the origin of the ID gaps (`compressRowIds` wrote nothing); that fix and ROW-DB-002's (`maxId` from the largest stored ID) have landed. ROW-UI specs have no tests yet. Opened to give the row-ID robustness work an arrow to run in, rather than touching `Database`'s row methods unscoped.

## References

### HLD
- docs/high-level-design.md — `## System Design` (segment list) and the `## Tenets` entry *row IDs are internal identity*

### LLD
- docs/intent/row-lifecycle/row-lifecycle-design.md

### EARS
- docs/intent/row-lifecycle/row-lifecycle-specs.md — facets `ROW-DB` (`Database` row mutation and ID assignment) and `ROW-UI` (Row menu actions, row editor, display refresh), mirroring the `COL-DB`/`COL-UI` split of the column-editing segment. Covers addition, copy, and row identity, including `deleteRow`'s and `compressRowIds`' renumbering; the deletion path's user-facing specs follow with that chunk of work.

### Tests
- tests/rowlifecycle/rowlifecycle_test.cpp — one test per ROW-DB spec, except ROW-DB-012, which has none yet. Fixtures come from `PbTest::buildStandardFile`, and `PobCorrupt::setIntAt` renumbers them when a test needs IDs with a gap. ROW-DB-010's crash runs in a child process (`--scenario`), following `tests/dbeditor_robustness/`.
- No ROW-UI tests yet; they need a harness that drives `RowEditor` and `DataModel`.

### Code
- src/portabase.cpp (`deleteRow`, `copyRow`, `deleteAllRows` — the Row menu actions only)
- src/viewdisplay.h, src/viewdisplay.cpp (`addRow`, `editRow`, `deleteRow`, `deleteAllRows`)
- src/roweditor.h, src/roweditor.cpp (`edit` when creating a row — the add and copy branches)
- src/datamodel.h, src/datamodel.cpp (`addRow`, `deleteRow`, `deleteAllRows`)
- src/view.cpp (`deleteAllRows`)
- src/database.h, src/database.cpp (`addRow`, `deleteRow`, `compressRowIds`, and the `maxId` initialization in `load`)

## Architecture

**Purpose:** Create, duplicate, and destroy the rows of a PortaBase file's data table, maintaining the format specification's `_id` invariant — unique and consecutive from 0 through one less than the row count — across every one of those operations.

**Key Components:**

1. `RowEditor` (roweditor.cpp) — the one-row dialog, shared with in-place row editing. Commits directly on accept rather than staging; cancelling simply never calls the database.
2. `Database` row primitives (database.cpp) — `addRow` mints the next `_id` and writes each value under the Metakit property derived from its column's ID and type; `deleteRow` removes a row and renumbers the survivors; `compressRowIds` re-densifies IDs after a batch.
3. `maxId` (database.cpp) — the in-memory cache of the largest ID in use, written by `load`, consumed by `addRow`, mutated by `deleteRow` and `compressRowIds`. Owning this one piece of shared state is why addition, deletion, and identity are one segment rather than three.
4. `DataModel` (datamodel.cpp) — the filter- and pagination-aware display refresh; a newly added row that fails the current filter produces no display change.

## Spec Coverage

| Facet | Specs | Implemented `[x]` | Active gap `[ ]` | With tests |
|---|---|---|---|---|
| ROW-DB | 12 | 9 | 3 (006, 010, 012) | 11 (`tests/rowlifecycle/`) |
| ROW-UI | 10 | 8 | 2 (003, 006) | 0 |

Eleven of the twelve ROW-DB specs have a test: the nine `[x]` pass, and ROW-DB-006 and ROW-DB-010 fail as expected (ROW-DB-010 by crashing a child process). ROW-DB-012 has no test yet. ROW-UI tests are not yet written; `[x]` there marks behavior the current code exhibits without a test to verify it.

## Key Findings

1. **`load` inferred `maxId` from the row count, not from the data** (fixed; ROW-DB-002). On a file whose IDs have gaps — real ones exist, reported by `portabase check` as CHK-DATA-003 — the next `addRow` minted an ID that was already present, after which every `data.Find(Id [id])` resolved to whichever duplicate was physically first and edited or deleted the wrong row. `load` now takes the largest stored `_id`. Files that already hold duplicate IDs still need the load-time repair (item 7).
2. **`deleteRow` does not check its `Find` result** (`database.cpp:1618`). A miss passes -1 to `RemoveAt`, which crashes the process (reproduced by ROW-DB-010's test).
3. **`compressRowIds` wrote nothing, and was the origin of the ID gaps** (fixed; ROW-DB-011). It wrote through the rows of a `SortOn` view, which Metakit silently ignores (`metakit/src/derived.cpp:257`, an assertion compiled out without `q4_CHECK`). Every bulk delete from `08a6402` (2011) until the fix left the survivors' IDs unchanged while setting `maxId` below them. Deleting the first 74 rows of a 99-row file leaves exactly `summer_camps.pob`'s IDs 74–98. It now maps each sorted row back to its `_data` index and writes there.
4. **`addRow`'s failure contract is broken at both ends.** The Sequence branch advances the column's stored default mid-loop (`database.cpp:1484-1486`), so a rejection at a later column burns a sequence number with no row added; and `RowEditor::edit` (`roweditor.cpp:73-75`) discards the returned error and reads a row-ID variable that `addRow` never assigns on the failure path, passing an indeterminate value to the image write and the display refresh.
5. **A copied row loses its image data but keeps its format string** (`imageselector.cpp:65-72`, `162-166`). `ImageSelector` treats an untouched field as unchanged and only writes bytes when changed, so copy produces a row whose format string names a format with no bytes behind it — a state no other code path creates, and contrary to the help text's description of copy.
6. **The code that wrote the known-affected file was unchanged until the ROW-DB-011 fix.** `summer_camps.pob` came from a build of `03ad038` (2020-06-06). Since then `src/view.cpp` and `src/roweditor.cpp` last changed in 2017, `src/datamodel.cpp` in 2015, `src/viewdisplay.cpp` in 2017, and `src/database.cpp`'s only changes are in `getView`, `reconcileAllView`, and `calculateAll`. The reproduction in finding 3 therefore describes how that file was produced.

## Work Required

### Must Fix

1. **Write the ROW-UI tests** once items 2–3 have their ROW-DB fixes. The ROW-DB tests are in `tests/rowlifecycle/`. The row editor's half of item 3 (ROW-UI-006) waits for these tests.
2. **Guard `deleteRow`'s `Find`** so an absent ID leaves `_data` and `maxId` unchanged (finding 2).
3. **Make a rejected `addRow` leave no trace** and stop `RowEditor::edit` acting on an unassigned row ID (finding 4).

### Should Fix

4. **Carry image data into a copied row** (finding 5). Specified as an active gap in the first EARS pass, since the intended behavior is settled; the implementation choice is open — load the source bytes when the field is set, or mark the field for write and defer the load until commit.
5. **Guard `View::deleteAllRows` against an empty batch** (ROW-DB-012), so a filter matching nothing leaves the IDs alone instead of running a compression pass that renumbers a file with gaps. Needs its failing test first.

### Nice to Have

6. **The broader row index-versus-ID audit** requested after COL-DB-010: `getId`, `getIndex`, `updateRow`, `editRow`, and the model-to-view index mapping have not been checked for the same confusion between a row's position and its identity.
7. **Open a load-time repair segment** once this one is complete. It would repair a file whose row IDs violate the invariant when the file is opened, and ask the user before saving the fix (direction recorded in the LLD's Deferred item 5). It needs the LID Scope in `CLAUDE.md` expanded to cover `Database::load`, `PortaBase::openFile`, and the CLI's import and export commands.
