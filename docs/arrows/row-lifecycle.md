# Arrow: row-lifecycle

The lifecycle of a data row — adding one (including copy-as-new), deleting one or many, and the `_id` assignment all of them depend on.

## Status

**MAPPED** — segment opened 2026-09-30 (git SHA `b94bbb9`). The LLD is drafted and reconciled with the code; no EARS specs exist yet, so nothing is verified against the implementation. Opened to give the row-ID robustness work an arrow to run in, rather than touching `Database`'s row methods unscoped.

## References

### HLD
- docs/high-level-design.md — `## System Design` (segment list) and the `## Tenets` entry *row IDs are internal identity*

### LLD
- docs/intent/row-lifecycle/row-lifecycle-design.md

### EARS
- docs/intent/row-lifecycle/row-lifecycle-specs.md — not yet drafted. Planned facets: `ROW-UI` (Row menu actions, row editor, display refresh) and `ROW-DB` (`Database` row mutation and ID assignment), mirroring the `COL-UI`/`COL-DB` split of the column-editing segment. The first drafting pass covers addition, copy, and row identity; the deletion path's specs follow with that chunk of work.

### Tests
- None yet. `tests/common/` already carries what this segment needs: `PbTest::buildStandardFile` for structurally valid fixtures and `PobCorrupt::setIntAt` for seeding an `_id` gap into `_data` directly.

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

No specs drafted yet. Coverage table follows the first EARS pass.

## Key Findings

1. **`load` infers `maxId` from the row count, not from the data** (`database.cpp:202`). On a file whose IDs are not a permutation of 0..n−1 — real ones exist, reported by `portabase check` as CHK-DATA-003 — the next `addRow` (`database.cpp:1419`) mints an ID that is already present, after which every `data.Find(Id [id])` resolves to whichever duplicate is physically first and edits or deletes the wrong row.
2. **`deleteRow` does not check its `Find` result** (`database.cpp:1609`). A miss does `RemoveAt(-1)` and then renumbers every ID above the absent one anyway, turning a no-op request into corruption.
3. **`compressRowIds` writes the sort key of a derived view while iterating it** (`database.cpp:1631-1640`). Structurally the most suspicious of the three, but hand-tracing it against the known-affected file's post-deletion state produces the correct result, so it is a hypothesis to test rather than a confirmed mechanism.
4. **`addRow`'s failure contract is broken at both ends.** The Sequence branch advances the column's stored default mid-loop (`database.cpp:1475-1477`), so a rejection at a later column burns a sequence number with no row added; and `RowEditor::edit` (`roweditor.cpp:73-75`) discards the returned error and reads a row-ID variable that `addRow` never assigns on the failure path, passing an indeterminate value to the image write and the display refresh.
5. **A copied row loses its image data but keeps its format string** (`imageselector.cpp:65-72`, `162-166`). `ImageSelector` treats an untouched field as unchanged and only writes bytes when changed, so copy produces a row whose format string names a format with no bytes behind it — a state no other code path creates, and contrary to the help text's description of copy.
6. **The code that wrote the known-affected file is still the current code.** `summer_camps.pob` came from a build of `03ad038` (2020-06-06). Since then `src/view.cpp` and `src/roweditor.cpp` last changed in 2017, `src/datamodel.cpp` in 2015, `src/viewdisplay.cpp` in 2017, and `src/database.cpp`'s only changes are in `getView`, `reconcileAllView`, and `calculateAll`. A reproduction attempt that fails therefore refutes the hypothesis rather than revealing an intervening fix.

## Work Required

### Must Fix

1. **Draft the EARS specs** (`ROW-UI`, `ROW-DB`) for addition, copy, and row identity — the segment has no specs, so nothing below the LLD is verifiable.
2. **Derive `maxId` from the largest stored `_id`** rather than the row count (finding 1). The robustness fix that stops a pre-existing gap from propagating into duplicate IDs.
3. **Guard `deleteRow`'s `Find`** so an absent ID leaves `_data` and `maxId` unchanged (finding 2).
4. **Make a rejected `addRow` leave no trace** and stop `RowEditor::edit` acting on an unassigned row ID (finding 4).

### Should Fix

5. **Reproduce the ID-gap origin**, distinguishing findings 1 and 3 rather than assuming either. Build a ~99-row fixture, delete a filtered subset through `View::deleteAllRows`, and assert the survivors compress to 0..n−1; separately, append rows to a file that already has a gap and assert no duplicate IDs appear. `portabase check`'s CHK-DATA-003 makes each assertion a one-liner.
6. **Carry image data into a copied row** (finding 5). Specified as an active gap in the first EARS pass, since the intended behavior is settled; the implementation choice is open — load the source bytes when the field is set, or mark the field for write and defer the load until commit.
7. **Guard `View::deleteAllRows` against an empty batch**, so a filter matching nothing does not run a full compression pass over every row in the file.

### Nice to Have

8. **The broader row index-versus-ID audit** requested after COL-DB-010: `getId`, `getIndex`, `updateRow`, `editRow`, and the model-to-view index mapping have not been checked for the same confusion between a row's position and its identity.
