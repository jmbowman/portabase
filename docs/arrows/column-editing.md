# Arrow: column-editing

The Edit Columns operation end-to-end — the staging dialog, the `Database` schema-mutation methods it drives, and the post-accept view maintenance and data-viewer refresh that complete the operation from the user's perspective.

## Status

**OK** — mapped 2026-07-10 (git SHA `4069734`); scope extended 2026-07-13 to the full user-visible operation (`PortaBase::editColumns` and the view-maintenance methods it drives); audited 2026-07-18 with all COL-UI/COL-DB specs implemented and covered by tests. Brownfield pilot for the agentic-scaffolding roadmap item.

## References

### HLD
- docs/high-level-design.md (project-level stub; too narrow a scope for project-wide content yet — see that doc's own notes)

### LLD
- docs/intent/column-editing/column-editing-design.md

### EARS
- docs/intent/column-editing/column-editing-specs.md

### Tests
- `tests/dbeditor/dbeditor_test.cpp` (QtTest) — behavior on valid files: add-column for every type category including enum and calculated columns (`COL-DB-002/003/006/007` happy path), invalid-name rejection (`COL-UI-002`, drives the nested warning-box sequence), staging/reorder/cancel (`COL-UI-001/004/005/009/010`), edit/rename/delete cascades (`COL-UI-003/006/007`, `COL-DB-004/005`), swap-rename (`COL-UI-014`), same-session interactions (`COL-UI-011/012/013`, `COL-UI-008`), ID reuse (`COL-DB-001`), delete-all (`COL-UI-015`)
- `tests/dbeditor_robustness/dbeditor_robustness_test.cpp` (QtTest) — resilience to inconsistent/corrupt file states: `_all` reconciliation (`COL-DB-008`), orphan-tolerant view load (`COL-DB-009`), the calc-recalc / row-ID crash that was the confirmed real-world root cause (`COL-DB-010`), and the view-structure crash reproducers (`COL-DB-007`). The crash cases run subprocess-isolated so a regression fails cleanly instead of segfaulting the run.
- `tests/common/` — shared fixture builder, raw-Metakit corruption seeder, and dialog-driving helpers

### Code
- src/dbeditor.h, src/dbeditor.cpp (`DBEditor` class, full files)
- src/columneditor.h, src/columneditor.cpp (`ColumnEditor` subdialog, full files)
- src/portabase.cpp (`PortaBase::editColumns` — the operation's entry point and post-accept sequence)
- src/database.h, src/database.cpp — schema-mutation methods (`setIndex`, `setDefault`, `addColumn`, `deleteColumn`, `renameColumn`, `updateDataFormat`), the view-maintenance methods the operation drives (`getView`, `addViewColumn`, `deleteViewColumn`, `setViewColumnSequence`, `reconcileAllView`), and calculated-column recalculation (`calculateAll`, `getRow`)

## Architecture

**Purpose:** Let a user redefine the column set of an existing PortaBase file (add/edit/delete/reorder columns, including calculated fields) without losing existing data, staging edits in a dialog until confirmed.

**Key Components:**
1. `DBEditor` (dbeditor.cpp) — the dialog. All in-dialog edits (`addColumn`, `editColumn`, `deleteColumn`, `moveUp`, `moveDown`) mutate only an in-memory staging view (`info`); nothing touches the real database until `applyChanges()` runs on dialog accept.
2. `DBEditor::applyChanges()` (dbeditor.cpp) — replays staged edits against the live `Database` in a fixed order: deletions → index/default tweaks → renames (two-pass, through temporary names) → additions → `updateDataFormat()` → `reconcileAllView()` → calculation updates.
3. `Database` schema methods — the actual mutation of column metadata (the `columns` view) and, for `addColumn`, writing default values into the live `data` view for every existing row; plus view maintenance (`reconcileAllView`, `getView`) and calculated-column recalculation (`calculateAll`).

## Spec Coverage

- **COL-UI** (Edit Columns dialog / staging behavior): COL-UI-001 through COL-UI-015, all `[x]` implemented.
- **COL-DB** (database schema mutation): COL-DB-001 through COL-DB-010, all `[x]` implemented. The originally-reported add-column crash (COL-DB-007) is resolved — root-caused to calculated-column recalculation over non-consecutive row IDs (COL-DB-010), with view-structure robustness added alongside (COL-DB-008/009).

## Key Findings

1. **Metakit has no separate schema migration step — this is intentional, longstanding design, not a bug.** `Database::addColumn` (`database.cpp`) writes new-column default values into every existing row of the live `data` view *before* `Database::updateDataFormat()` (called once after the whole addition loop in `applyChanges()`) regenerates the format string used by future load operations. Per the maintainer: Metakit is not SQL — the "format" is essentially a descriptive string for future parsing, not a gate on what properties can currently be written to a row. This ordering has worked this way for ~25 years and is intentional, Metakit-appropriate design, not a defect. (The originally-reported crash was unrelated — see finding 2.)
2. **The reported add-column crash (COL-DB-007) is root-caused, reproduced, and fixed.** The real crashing file (`summer_camps.pob`) has a calculated column and non-consecutive row IDs (`portabase check` reports CHK-DATA-003). On any accepted Edit Columns dialog, `applyChanges` recalculated the calc column via `Database::calculateAll`, which passed a *physical row index* to `getRow`, which treats it as a *row ID* — the Find missed on the ID gap and crashed. Column-type-independent and cross-platform exactly as reported, because the crash is in recalculating the pre-existing calc column, not the added one. Fixed by COL-DB-010 (address rows by `_id`); verified against the real file. The separately-hypothesized view-structure crashes (COL-DB-008/009) were real but *not* the maintainer's crash (no such corruption in 63 scanned files); hardened anyway for older files.
3. **`addColumn`'s `id` parameter defaults to -1** (database.h), triggering an id-gap-finding scan when the caller (always `DBEditor::applyChanges`, in this codebase) doesn't supply one explicitly.

## Work Required

### Must Fix
(none — all COL-UI and COL-DB specs implemented; the full `tests/dbeditor_robustness/` suite passes.)

### Should Fix
1. **Row-ID robustness outside this segment's scope**, likely a new segment with a scope-extension decision: the *origin* of non-consecutive row IDs (maintainer suspects calculated-field handling; the affected file was appended-to, not deletion/import-corrupted), plus `Database::deleteRow`'s renumber-without-removal, `load()`'s `maxId = GetSize() - 1`, and `commit()`'s ignored return values (see `docs/wiki/dbeditor-code-review-followups.md`).
2. **Broader row index/ID audit**: `getRow()` was audited clean apart from the COL-DB-010 bug; the maintainer requested extending it to all functions taking a row index or ID (`getId`, `getIndex`, `updateRow`, `deleteRow`, `editRow`, model↔view mapping).
3. **Widget-layer coverage**: the suites drive `Database`/`DBEditor` but not `ViewDisplay`/`DataModel`; the viewer refresh around `updateDataFormat()`'s storage restructure — including the `viewer->setDatabase` vs `viewAllColumns` branch in `PortaBase::editColumns` — is not exercised by the headless tests (no COL spec mandates viewer-display behavior) and is best verified by running the app: create a new file and define its first columns (should show the data view), and edit columns on an existing file (should refresh).

### Nice to Have
(none)
