# Arrow: column-editing

The "Edit Columns" dialog and the `Database` schema-mutation methods it drives — adding, deleting, renaming, and reordering the columns of an existing PortaBase file.

## Status

**MAPPED** — mapped 2026-07-10 (git SHA `4069734`). Brownfield pilot for the agentic-scaffolding roadmap item; not yet audited against tests (none exist yet).

## References

### HLD
- docs/high-level-design.md (not yet written)

### LLD
- docs/intent/column-editing/column-editing-design.md (not yet written)

### EARS
- docs/intent/column-editing/column-editing-specs.md (not yet written)

### Tests
- none exist yet

### Code
- src/dbeditor.h, src/dbeditor.cpp (`DBEditor` class, full files)
- src/database.h, src/database.cpp lines 555–911 (`setIndex`, `setDefault`, `addColumn`, `deleteColumn`, `renameColumn`, `updateDataFormat`) plus `addViewColumn` (database.cpp:1719), called directly from `addColumn`

## Architecture

**Purpose:** Let a user redefine the column set of an existing PortaBase file (add/edit/delete/reorder columns, including calculated fields) without losing existing data, staging edits in a dialog until confirmed.

**Key Components:**
1. `DBEditor` (dbeditor.cpp) — the dialog. All in-dialog edits (`addColumn`, `editColumn`, `deleteColumn`, `moveUp`, `moveDown`) mutate only an in-memory staging view (`info`); nothing touches the real database until `applyChanges()` runs on dialog accept.
2. `DBEditor::applyChanges()` (dbeditor.cpp:499) — replays staged edits against the live `Database` in a fixed order: deletions → index/default tweaks → renames → additions → `updateDataFormat()` → calculation updates.
3. `Database` schema methods (database.cpp:555–911) — the actual mutation of column metadata (the `columns` view) and, for `addColumn`, writing default values into the live `data` view for every existing row.

## Spec Coverage

Not yet generated — specs don't exist yet (next artifact in this pass).

## Key Findings

1. **Metakit has no separate schema migration step — this is intentional, longstanding design, not a bug.** `Database::addColumn` (database.cpp:698) writes new-column default values into every existing row of the live `data` view *before* `Database::updateDataFormat()` (database.cpp:898, called once after the whole addition loop in `applyChanges()`) regenerates the format string used by future load operations. Per the maintainer: Metakit is not SQL — the "format" is essentially a descriptive string for future parsing, not a gate on what properties can currently be written to a row. This ordering has worked this way for ~25 years and should **not** be treated as a lead for the known crash bug.
2. **The known crash (add column → Save → crash, cross-platform, column-type-independent) is a more recent regression** — per the maintainer, likely introduced within roughly the last 6–8 years, not inherent to the decades-old design in Finding 1. Root cause not yet identified; worth git-archaeology on this file range once specs exist to check behavior against.
3. **`addColumn`'s `id` parameter defaults to -1** (database.h:82-83), triggering an id-gap-finding scan (database.cpp:702-716) when the caller (always `DBEditor::applyChanges`, in this codebase) doesn't supply one explicitly.

## Work Required

### Must Fix
1. Diagnose and fix the add-column-then-save crash (not yet root-caused; see Key Finding 2). Affects EARS spec(s) to be written in the next artifact.

### Should Fix
(none identified yet — brownfield mapping in progress)

### Nice to Have
(none identified yet)
