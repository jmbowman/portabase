# Column Editing — EARS Specs

## COL-UI (Edit Columns dialog / staging behavior)

- [x] **COL-UI-001**: The system shall stage all column additions, edits, deletions, and reorderings in memory while the Edit Columns dialog is open, without modifying the underlying database file.
- [x] **COL-UI-002**: When the user presses "Add", the system shall prompt for a new column's name, type, and default value, and shall reject invalid names or default values without closing the subdialog.
- [x] **COL-UI-003**: When the user presses "Edit" on an existing column, the system shall allow changing the column's name and default value but not its data type.
- [x] **COL-UI-004**: When the user presses "Delete" on a column, the system shall remove it from the staged column list without immediately modifying the database file.
- [x] **COL-UI-005**: When the user presses "Up" or "Down" on a column, the system shall swap its staged position with the adjacent column's.
- [x] **COL-UI-006**: When the user renames a column that is referenced by a calculated column's formula, the system shall update that formula to use the new name.
- [x] **COL-UI-007**: When the user deletes a column that is referenced by a calculated column's formula, the system shall clear or invalidate that formula's reference to the deleted column.
- [x] **COL-UI-008**: When the Edit Columns dialog is confirmed ("OK"/"Done"), the system shall apply all staged changes to the database in this order: deletions, then index/default changes to surviving columns, then renames, then additions, then calculated-column updates.
- [x] **COL-UI-009**: When the Edit Columns dialog is cancelled, the system shall discard all staged changes and leave the database's column format unmodified.
- [x] **COL-UI-010**: When a column is deleted from the staged list, the system shall renumber the remaining staged columns' positions to a contiguous sequence starting at zero, based on each column's current staged position, regardless of any reordering ("Up"/"Down") performed earlier in the same dialog session.

## COL-DB (Database schema mutation)

- [x] **COL-DB-001**: When a column is added, the system shall assign it a unique integer ID, reusing the lowest available gap left by a previously-deleted column's ID if one exists.
- [x] **COL-DB-002**: When a column is added, the system shall populate a type-appropriate default value for that column in every existing data row.
- [x] **COL-DB-003**: When a column is added, the system shall register it in the "_all" view (the view listing all columns) at the position specified by the caller.
- [x] **COL-DB-004**: When a column is deleted, the system shall remove it from every view, sorting, and filter that references it, and shall delete its calculation definition if it is a calculated column.
- [x] **COL-DB-005**: When a column is renamed, the system shall update references to it in every view, sorting, filter, and calculation that uses it.
- [x] **COL-DB-006**: When one or more columns have been added, deleted, or renamed, the system shall regenerate the data table's format string to reflect the current column set.
- [ ] **COL-DB-007**: When a column is added to an existing (previously-saved) database file and the Edit Columns dialog is saved, the system shall apply the change without crashing. **Active gap** — known crash bug, reproduces on both Android and Ubuntu regardless of the new column's type; root cause not yet identified (see `column-editing-design.md` § Open Questions & Future Decisions).
