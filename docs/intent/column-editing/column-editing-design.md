---
parent: high-level-design
prefix: COL
---

# Column Editing

## Context and Design Philosophy

Each PortaBase file has a user-defined tabular format: a sequence of typed, named, defaulted columns. This component lets a user redefine that format on an existing file — add, edit, delete, or reorder columns — without losing existing data, and without applying any change until the user explicitly confirms.

The user-facing behavior is documented in `resources/help/columns_editor.txt` (the "Edit Columns" dialog help text): edits are staged and only take effect on "OK"; "Cancel" discards them entirely.

## Staging Model (DBEditor)

`DBEditor` (`src/dbeditor.cpp`/`.h`) never touches the live `Database` while the dialog is open. All edits — `addColumn()`, `editColumn()`, `deleteColumn()`, `moveUp()`/`moveDown()` — mutate only an in-memory Metakit view (`info`) that mirrors the column list, tracking each row's original position (`ceOldIndex`, -1 for new columns) and current position (`ceNewIndex`). `updateTable()` re-renders the displayed list from `info` after every edit. Only `applyChanges()`, invoked on dialog accept, touches the real `Database`.

`info`'s rows are appended in position order once, at construction (`edit()`) and by each `addColumn()` call — nothing ever changes a row's *physical* position in the view afterward. `moveUp()`/`moveDown()` reorder columns by swapping the `ceNewIndex` *value* between two rows, not by moving either row physically. Physical row order and `ceNewIndex` order therefore only coincide until the first reorder in a session; any later code that needs "the columns in their current staged order" must sort or filter on the `ceNewIndex` value, never assume physical position matches it.

## Applying Changes to the Database

`DBEditor::applyChanges()` (dbeditor.cpp:499) replays the staged edits against `Database` in a fixed order:

1. Deletions (`Database::deleteColumn`)
2. Index and default-value changes to surviving original columns (`setIndex`, `setDefault`)
3. Renames (`Database::renameColumn`)
4. Additions (`Database::addColumn`)
5. `Database::updateDataFormat()` — once, after all additions
6. Calculated-column updates (`Database::updateCalc`)

`Database::addColumn` (database.cpp:698) does more than register the new column in the `columns` metadata view: it also writes a type-appropriate default value into every existing row of the live `data` view immediately, using a Metakit property keyed to the new column's ID — before `updateDataFormat()` regenerates the format string that later load operations use to parse the file.

## Decisions & Alternatives

| Decision | Chosen | Alternatives Considered | Rationale |
|----------|--------|--------------------------|-----------|
| When edits take effect | Staged in-memory, applied atomically on dialog accept | Apply each edit immediately as made | Matches documented user-facing behavior (`columns_editor.txt`): Cancel must discard all changes cleanly. |
| Order of operations in `applyChanges()` | Deletions → index/default → renames → additions → `updateDataFormat()` → calc updates | [inferred] Unclear if any other order was considered | [inferred] Deletions first likely avoids transient name/index collisions between old and new column sets during the replay. |
| Writing new-column data into `data` rows before `updateDataFormat()` | Immediate per-column write during `addColumn`, format string updated once afterward, for all additions | [inferred] Update format string per-column instead | Confirmed non-inferred, from the maintainer directly: Metakit is not SQL — the format string just describes the row layout for future load/parse operations, it does not gate what properties can be written to a row right now. This ordering is original, ~25-year-old design, not a later shortcut. |
| Column ID allocation | Scan existing IDs (sorted) for the first gap, reuse it; otherwise increment | [inferred] Monotonically increasing counter, never reused | [inferred] Likely keeps IDs compact after column deletions, though the actual motivation isn't visible in the code itself. |
| Staged-position renumbering after delete | Renumber every remaining row whose `ceNewIndex` value exceeds the deleted row's `ceNewIndex` value | Renumber every remaining row at a physical position at or after the deleted row's physical position (original implementation) | The physical-position approach assumed physical row order in `info` always matches `ceNewIndex` order — true only until the first `moveUp`/`moveDown` in the session. Renumbering by value holds regardless of any prior reordering. |

## Open Questions & Future Decisions

### Resolved
1. ✅ Whether the write-before-format-update ordering in `addColumn`/`updateDataFormat` is a bug: **no** — confirmed intentional, longstanding Metakit-appropriate design (see Decisions table).
2. ✅ Whether reordering columns (`moveUp`/`moveDown`) and then deleting a different column in the same dialog session could corrupt the persisted column position sequence: **yes** — see the staged-position renumbering row in the Decisions table above. Covered by `COL-UI-010`.

### Deferred
1. **Root cause of the known crash**: existing file → Edit Columns → add a column → Save → crash. Reproduces on both Android and Ubuntu; column type doesn't appear to matter. Per the maintainer, likely introduced as a regression sometime in roughly the last 6–8 years, not present in the original design. Not yet root-caused — a QtTest regression suite (`tests/dbeditor/dbeditor_test.cpp`) covering a realistic multi-typed, populated fixture does not reproduce it, so the trigger is something about real-world file history not yet captured synthetically. Needs a real crashing file to instrument against.
2. **Automated test coverage** now exists for this component (`tests/dbeditor/dbeditor_test.cpp`, QtTest-based) but is not comprehensive — it covers add-column-to-existing-file (all base types) and the reorder-then-delete interaction (`COL-UI-010`); other staged operations (edit, rename, calculated-column interactions) are not yet covered.

## References

- Arrow doc: `docs/arrows/column-editing.md`
- Help documentation: `resources/help/columns_editor.txt`
- Code: `src/dbeditor.h`, `src/dbeditor.cpp`; `src/database.h`, `src/database.cpp` (lines 555–911, plus `addViewColumn` at 1719)
