---
parent: high-level-design
prefix: COL
---

# Column Editing

## Context and Design Philosophy

Each PortaBase file has a user-defined tabular format: a sequence of typed, named, defaulted columns. This component lets a user redefine that format on an existing file — add, edit, delete, or reorder columns — without losing existing data, and without applying any change until the user explicitly confirms.

The segment covers the operation end-to-end as the user experiences it: the menu action (`PortaBase::editColumns`, `src/portabase.cpp`), the staging dialog (`DBEditor` and its per-column subdialog `ColumnEditor`), the `Database` schema-mutation methods the dialog drives on accept, and the post-accept view maintenance and data-viewer refresh. A crash anywhere in that chain is a failure of this operation from the user's perspective, wherever the faulting line lives.

The user-facing behavior is documented in `resources/help/columns_editor.txt` (the "Edit Columns" dialog help text): edits are staged and only take effect on "OK"; "Cancel" discards them entirely. The on-disk structures the operation mutates are specified in the [PortaBase format specification](../../wiki/format.md) — notably the `_columns`, `_data`, `_views`, and `_viewcolumns` views, and the invariant that a view named `_all` always exists containing all columns in `_cindex` order.

## Staging Model (DBEditor)

`DBEditor` (`src/dbeditor.cpp`/`.h`) never touches the live `Database` while the dialog is open. All edits — `addColumn()`, `editColumn()`, `deleteColumn()`, `moveUp()`/`moveDown()` — mutate only an in-memory Metakit view (`info`) that mirrors the column list, tracking each row's original position (`ceOldIndex`, -1 for new columns) and current position (`ceNewIndex`). `updateTable()` re-renders the displayed list from `info` after every edit. Only `applyChanges()`, invoked on dialog accept, touches the real `Database`.

`info`'s rows are appended in position order once, at construction (`edit()`) and by each `addColumn()` call — nothing ever changes a row's *physical* position in the view afterward. `moveUp()`/`moveDown()` reorder columns by swapping the `ceNewIndex` *value* between two rows, not by moving either row physically. Physical row order and `ceNewIndex` order therefore only coincide until the first reorder in a session; any later code that needs "the columns in their current staged order" must sort or filter on the `ceNewIndex` value, never assume physical position matches it.

## Applying Changes to the Database

`DBEditor::applyChanges()` (`dbeditor.cpp`) replays the staged edits against `Database` in a fixed order:

1. Deletions (`Database::deleteColumn`)
2. Index and default-value changes to surviving original columns (`setIndex`, `setDefault`)
3. Renames (`Database::renameColumn`), replayed through unique temporary names in two passes so cyclic/swapped renames resolve correctly (see COL-UI-014)
4. Additions (`Database::addColumn`)
5. `Database::updateDataFormat()` — once, after all additions
6. `Database::reconcileAllView()` — bring the `_all` view into exact correspondence with the resulting column set
7. Calculated-column updates (`Database::updateCalc`)

`Database::addColumn` (`database.cpp`) does more than register the new column in the `columns` metadata view: it also writes a type-appropriate default value into every existing row of the live `data` view immediately, using a Metakit property keyed to the new column's ID — before `updateDataFormat()` regenerates the format string that later load operations use to parse the file. It also appends the new column to the `_all` view via `addViewColumn("_all", name)`; `addViewColumn` silently no-ops if the named view does not exist.

## Post-Accept Sequence (PortaBase::editColumns)

When the dialog is accepted, `PortaBase::editColumns()` (`portabase.cpp`) runs the full operation:

1. `viewer->closeView()` — release the current data view before the schema shifts under it.
2. `DBEditor::applyChanges()` — replays the staged edits (see above) and, as its final step, calls `Database::reconcileAllView()` to bring the `_all` view back into exact correspondence with the resulting column set.
3. Display refresh, branched on whether the file had any views *before* this operation (captured before step 2, since `reconcileAllView` always creates `_all`): a brand-new file (no prior views) gets a full viewer initialization via `viewer->setDatabase()`; an existing file gets `viewAllColumns()`, which loads `_all` through `Database::getView("_all")`.
4. `showDataViewer()` and `setEdited(true)` — refresh the display and mark the file dirty for save.

### Robustness to inconsistent file state

Per the HLD tenet *never trust file-derived state*, the operation must not crash on files whose view structures carry historical inconsistencies (decades of history across versions and past bugs). Metakit's `Find()` returns -1 on a miss and builds a cursor that crashes lazily on first property access rather than failing fast, so every file-derived lookup on this path is guarded:

- `Database::reconcileAllView` (COL-DB-008) makes `_all` exactly the current column set on every accept — recreating it if missing, dropping entries for columns that no longer exist, adding entries it lacks, and reordering to match column positions. Because it runs inside `applyChanges` before any later step reads `_all`, the downstream `setViewColumnSequence`/`getView` calls always operate on a consistent `_all`.
- `Database::getView` (COL-DB-009) omits any view-column entry whose name no longer resolves to a column, instead of indexing the `columns` view at a `Find()` miss.
- `Database::calculateAll` (COL-DB-010) addresses each data row by its actual `_id` rather than by physical position, so recalculating a calculated column does not crash on a file whose row IDs contain gaps — this was the confirmed cause of the originally-reported crash (see Open Questions § Resolved).

About ten further unguarded `Find()` sites in the schema-mutation methods remain catalogued in `docs/wiki/dbeditor-code-review-followups.md`; they share the failure mode but are not on a currently-reproduced crash path.

## Test Fixtures Must Be Structurally Valid Files

A fixture built by direct `Database` API calls is not automatically a valid PortaBase file. Per the [PortaBase format specification](../../wiki/format.md), a real file always has: an `_all` view listing every column in `_cindex` order, an `_allrows` filter, and a `_global` row whose `_gview` names an existing view. `Database`'s constructor alone does not create the `_all` view — `PortaBase::editColumns()` (or import) does — so a fixture assembled from bare `addColumn()` calls has *no* views while its `_global` row claims the current view is `_all`; the real application would crash opening such a file. Tests for this segment build fixtures through the same call sequence the application uses (`PbTest::buildStandardFile` in `tests/common/`, which creates `_all`, a user view, a sorting, a filter, and a calculated column), and exercise the full post-accept sequence rather than stopping at `applyChanges()`. Crash-reproducing cases seed a single inconsistency (a missing `_all`, an orphaned view reference, or a row-ID gap) into a valid fixture with the raw-Metakit corruptor in `tests/common/`.

The fixture builder and the raw-Metakit corruption seeder (for reproducing historical-corruption states) live in `tests/common/`, shared with the integrity-check segment as test infrastructure owned by neither segment.

## Decisions & Alternatives

| Decision | Chosen | Alternatives Considered | Rationale |
|----------|--------|--------------------------|-----------|
| When edits take effect | Staged in-memory, applied atomically on dialog accept | Apply each edit immediately as made | Matches documented user-facing behavior (`columns_editor.txt`): Cancel must discard all changes cleanly. |
| Order of operations in `applyChanges()` | Deletions → index/default → renames → additions → `updateDataFormat()` → `reconcileAllView()` → calc updates | A different replay order (e.g. additions or renames before deletions) | Chosen to avoid name collisions: deletions and renames run before additions so that any existing column name a later operation needs is freed before that operation tries to use it. `reconcileAllView` runs once the column set is final; calc updates run last, against the finalized columns. |
| Cyclic/swapped renames | Replay renames through unique underscore-prefixed temporary names in two passes (all-to-temp, then all-to-final) | Direct sequential name-based rename | `Database::renameColumn` is a name-based find-and-replace; a direct replay of a cycle (e.g. two columns swapping names) transiently duplicates a name, so the next rename resolves to the wrong column by first-match. Two passes through temp names break every cycle. There is no cheaper correct option, since references in views/sortings/filters/calculations are also stored by name and must cascade through the same rename. |
| Writing new-column data into `data` rows before `updateDataFormat()` | Immediate per-column write during `addColumn`, format string updated once afterward, for all additions | Update the format string per-column instead (not seriously considered) | Metakit is not SQL — the format string just describes the row layout for future load/parse operations, it does not gate what properties can be written to a row right now, so adjusting the row data and then the format string is the standard Metakit pattern. A per-column format update would be more work for no real benefit. This ordering is original, ~25-year-old design, not a later shortcut. |
| Column ID allocation | Scan existing IDs (sorted) for the first gap, reuse it; otherwise increment | Monotonically increasing counter, never reused | Intended to avoid exhausting the integer ID space — the primary-key-exhaustion problem the maintainer had hit with SQL databases at work. Reusing the lowest freed gap keeps the column IDs in use bounded by the number of columns present at once, so the space runs out only if that many columns exist simultaneously; a monotonic counter instead climbs with every add/delete cycle, opening gaps and marching toward exhaustion regardless of how few columns exist at any moment. |
| Staged-position renumbering after delete | Renumber every remaining row whose `ceNewIndex` value exceeds the deleted row's `ceNewIndex` value | Renumber every remaining row at a physical position at or after the deleted row's physical position (original implementation) | The physical-position approach assumed physical row order in `info` always matches `ceNewIndex` order — true only until the first `moveUp`/`moveDown` in the session. Renumbering by value holds regardless of any prior reordering. |
| `_all` view maintenance on dialog accept | Reconcile `_all` to exactly the current column set in position order — create if missing, add absent entries, drop orphans, renumber (`Database::reconcileAllView`, called from `applyChanges`) | Only create `_all` when the file has zero views and otherwise trust it to be complete; fail loudly on inconsistency | Files with a missing or stale `_all` exist in the wild (decades of history, past bugs). Reconciliation makes accept self-healing, while the integrity checker still reports the anomaly for diagnosis. Living in `Database` (not `DBEditor`) lets `PortaBase::editColumns` and any future caller reuse it; `addColumn` still appends via `addViewColumn` so non-dialog callers keep working. |
| `addViewColumn` behavior when the target view doesn't exist | Silent no-op | Create the view; fail loudly | A tolerant primitive that non-dialog callers rely on; the operation-level guarantee is provided by `reconcileAllView` on accept rather than by each primitive, so `addViewColumn`'s no-op no longer leaves `_all` inconsistent after an Edit Columns operation. |
| Dialog instance lifetime | Single-use: a fresh `DBEditor` is constructed per invocation and never re-launched | Resettable, reusable dialog instance | Simpler; the sole call site constructs a new instance each time, so `edit()` does not need to reset staged state. |
| Emptying the column set | Allowed — deleting every column returns the file to the valid zero-column state | Forbid deleting the last column | Zero columns is the starting state of every newly created file; returning to it is harmless (if uncommon). |

## Open Questions & Future Decisions

### Resolved
1. ✅ Whether the write-before-format-update ordering in `addColumn`/`updateDataFormat` is a bug: **no** — confirmed intentional, longstanding Metakit-appropriate design (see Decisions table).
2. ✅ Reordering columns (`moveUp`/`moveDown`) then deleting a different column in the same session corrupting the persisted position sequence: **yes**, fixed — see the staged-position renumbering row in the Decisions table. Covered by `COL-UI-010`.
3. ✅ **Root cause of the originally-reported add-column crash (COL-DB-007).** The crashing file — `summer_camps.pob`, a real file the maintainer reproduced the crash against locally (not committed to the repository) — has a calculated column and non-consecutive row IDs (`portabase check` flags it, CHK-DATA-003). `applyChanges` recalculates every calculated column via `Database::calculateAll`, which looped over *physical row index* and passed it to `getRow`, which treats its argument as a *row ID* — the `Find` missed on the ID gap and the -1 cursor crashed lazily. This explains every property of the original report: column-type-independent (the crash is in recalculating the *pre-existing* calc column, not the added one), cross-platform (pure data-layer logic), and "specific" (needs a calc column *and* an ID gap). **Fixed (COL-DB-010)**: `calculateAll` addresses each row by its actual `_id`. Verified against the real file (survives, values correct) and captured synthetically in `tests/dbeditor_robustness/`. The fix does not repair the underlying ID gap — see Deferred.
4. ✅ **View-structure crashes on inconsistent files (COL-DB-008/009).** A missing `_all` view, or a `_viewcolumns` entry naming a column that no longer exists, crashed the post-accept sequence's unguarded `Find()` lookups. Fixed by `Database::reconcileAllView` (rebuilds `_all` to match the column set on every accept) and `Database::getView` omitting unresolvable entries. Not observed in the maintainer's 63 real files but latent for older files; reproducers in `tests/dbeditor_robustness/`.
5. ✅ **Cyclic/swapped renames (COL-UI-014).** A direct sequential rename replay mis-targeted references when the staged renames formed a cycle. Fixed by replaying through unique temporary names in two passes — see the Decisions table.

### Deferred
1. **Origin of the row-ID gaps** (outside this segment's scope): the mechanism that first writes non-consecutive `_id` values is not yet found. The maintainer reports the affected file was mostly appended-to (few/no row deletions) and was never CSV-*imported* (the sibling `.csv` was an export), making a defect in calculated-field handling the leading suspect. `Database::deleteRow`'s renumber-without-removal and `load()`'s `maxId = GetSize() - 1` are related robustness gaps recorded in `docs/wiki/dbeditor-code-review-followups.md`. COL-DB-010 makes recalculation *survive* such files; it neither prevents the gap from forming nor repairs an existing one.
2. **Orphaned `_viewcolumns` rows in user-defined views (not `_all`)**: `getView` omits unresolvable references at load (COL-DB-009) and accept reconciles `_all` (COL-DB-008), but the view-editing/saving paths for user views have not been audited for lingering orphans. Purging them at those mutation points is permitted auto-repair under the HLD tenet (unambiguous, lossless) if a defect is found there.
3. **Widget-layer coverage**: the suites drive `Database`/`DBEditor` but not `ViewDisplay`/`DataModel`; the viewer refresh around `updateDataFormat()`'s storage restructure (including the `viewer->setDatabase` vs `viewAllColumns` branch in `PortaBase::editColumns`) is not exercised by the headless tests and needs manual verification.

## References

- Arrow doc: `docs/arrows/column-editing.md`
- Help documentation: `resources/help/columns_editor.txt`
- [PortaBase format specification](../../wiki/format.md)
- Code: `src/dbeditor.h`, `src/dbeditor.cpp` (`applyChanges`, including the two-pass rename replay); `src/columneditor.h`, `src/columneditor.cpp`; `src/portabase.cpp` (`editColumns`); `src/database.h`, `src/database.cpp` (schema mutation `setIndex`/`setDefault`/`addColumn`/`deleteColumn`/`renameColumn`/`updateDataFormat`; view maintenance `getView`/`addViewColumn`/`deleteViewColumn`/`setViewColumnSequence`/`reconcileAllView`; calculated-column recalculation `calculateAll`/`getRow`)
- Known lower-priority findings on this code: `docs/wiki/dbeditor-code-review-followups.md`
