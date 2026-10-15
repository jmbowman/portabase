---
parent: high-level-design
prefix: ROW
---

# Row Lifecycle

## Context and Design Philosophy

A PortaBase file's data rows are created, copied, and destroyed one at a time by the user, or in bulk by a filtered delete. This segment covers those operations end-to-end as the user experiences them — the Row menu actions, the row editor dialog, the `Database` methods that mutate the `_data` view, and the display refresh that follows — together with the row-identity scheme all of them depend on.

Row identity is the thread that ties the operations together. Every `_data` row carries an `_id`, and the [PortaBase format specification](../../wiki/format.md) states the invariant: IDs are unique and consecutive from 0 through one less than the row count, with rows renumbered when one is deleted. Adding a row mints the next ID, deleting one renumbers the survivors, and the in-memory `maxId` cache is written and read by both. A defect in any one of them is visible as corruption in the others, so they are designed as one unit.

Per the HLD tenet *row IDs are internal identity*, `_id` is addressing, not data. Nothing the user sees is derived from it, which is what makes wholesale renumbering on delete an acceptable price for a dense ID space. A user-facing number that must survive deletions is what the Sequence column type is for.

The user-facing behavior is documented in `resources/help/row_editor.txt` and the Row menu section of `resources/help/data_viewer.txt`: adding shows the column defaults, copying pre-fills from the selected row instead, cancelling aborts the addition, and "Delete Rows In Filter" deletes everything the current filter includes.

## Row Identity and the `maxId` Cache

`Database` caches the largest row ID in use in `maxId` rather than deriving it per operation:

| Site | Effect on `maxId` and `_id` |
|---|---|
| `Database::load` | Sets `maxId = data.GetSize() - 1` — inferred from the row *count* |
| `Database::addRow` | Writes `maxId + 1` into the new row, then increments `maxId` |
| `Database::deleteRow(id, true)` | Removes the row, decrements every ID greater than `id`, decrements `maxId` |
| `Database::deleteRow(id, false)` | Removes the row only; the caller is responsible for compressing afterwards |
| `Database::compressRowIds` | Should rewrite each ID to its rank in `_id` order (currently writes nothing; see below), then sets `maxId = size - 1` |

The `compressRowIds` flag on `deleteRow` exists so a bulk delete pays the renumbering cost once instead of once per row: `View::deleteAllRows` collects the matching IDs, deletes each with renumbering suppressed, then compresses a single time. Because compression renumbers from 0 regardless of the IDs it starts from, a bulk delete on a file whose IDs have gaps also closes them. The file is marked edited, as after any deletion, so the corrected IDs reach disk only when the user saves.

**Current state diverges from that intent in three places**, all of which assume the invariant they are meant to maintain:

1. `load` accepts a file whose IDs violate the invariant as it finds it, and infers `maxId` from the row count, not from the data. Such files exist in the wild — `portabase check` reports the condition as CHK-DATA-003 — and on one the next `addRow` mints an ID that is already present. Every subsequent `data.Find(Id [id])` lookup resolves to whichever duplicate is physically first, so an edit or a delete silently lands on the wrong row. Deriving `maxId` from the largest `_id` actually stored keeps the cache correct on any file whose IDs are unique, gaps included, and the deletion paths keep it correct from then on. Duplicate IDs are beyond what the cache can protect: once a delete removes one of two rows sharing the largest ID, the decremented `maxId` falls back onto the survivor's ID. Duplicates are left to the load-time repair. Repairing such a file's IDs when it is loaded is separate, planned work outside this segment (see *Open Questions & Future Decisions*).
2. `deleteRow` does not check the result of `data.Find`. A miss passes -1 to `RemoveAt`, which crashes the process. Even if it didn't crash, the method would go on to renumber every ID above the absent one, shifting IDs with no matching removal. A request to delete an ID that is not present must instead leave `_data` and `maxId` exactly as they were.
3. `compressRowIds` writes nothing. It assigns the new IDs through the rows of a `data.SortOn(Id)` view, but the rows of a Metakit sorted view are read-only: `c4_FilterSeq::Set` is only an assertion, and assertions are compiled out unless the library is built with `q4_CHECK`, which PortaBase's build does not set. Every write is silently dropped. Meanwhile `maxId` is still set to the surviving row count minus one. Since bulk deletion took this form in 2011, every "Delete Rows In Filter" has left the surviving rows' IDs as they were and pointed `maxId` below them, so the next rows added duplicate surviving IDs. Renumbering must write to `_data` itself, reading the order from the sorted view without writing through it.

## Adding a Row

The Row ▸ Add action runs the operation through four stages:

1. `ViewDisplay::addRow` opens `RowEditor` with a row ID of -1.
2. `RowEditor::edit` populates every field from `Database::getDefault` and loops on `exec()` until either the dialog is cancelled or `isValid()` passes, so an invalid integer, decimal, or time value re-shows the dialog with a warning rather than closing it.
3. On accept, `Database::addRow` re-validates every value (`isValidValue`), and on any failure returns an error message without touching `_data`. It also rejects a value list whose length does not match the column count. Otherwise it builds the row and appends it with `data.Add`.
4. `DataModel::addRow` refreshes the display and `ViewDisplay` marks the file edited.

`Database::addRow` writes each value under the Metakit property derived from the column's ID and type, which for several types means more than one property: decimal and calculation values are stored as both a float (for sorting, filtering, and statistics) and the string the user entered (for display); an enum value is stored as both its option text and its option index; an image is stored as bytes plus a string naming the format. This is the layout the format specification defines, and `addRow` is one of the two places that must produce it in full.

Two values are not taken from the dialog as displayed:

- **Sequence columns** draw from the column's stored default, not from the dialog and not from `_id`. `addRow` writes the current default into the row and increments the stored default, so sequence numbers are allocated by the column and survive the renumbering that deletions inflict on `_id`.
- **Image bytes** are written in a second phase. `addRow` stores the format string with empty bytes, and `RowEditor::edit` then calls `ImageSelector::saveImage` with the ID of the row just created, because writing the bytes requires a row that already exists.

A new row is appended physically; where it appears to the user is determined by the current view's sorting and filter, resolved by the display refresh rather than by the insertion.

**A rejected addition must leave nothing behind, and its caller must not treat it as a row.** `addRow` reports failure by returning a message and leaves the file exactly as it was: no row appended, no Sequence value advanced, no enum option added, and the row-ID out-parameter left unwritten. The row editor responds to a rejection the way it responds to its own validation failures — it shows the message and re-shows the dialog with the user's values intact, so the user can correct the value or cancel. **Current state diverges on both halves of that contract.** The Sequence branch advances the column's stored default the moment it reaches that column, and an import that permits new enum options adds each one as its value validates, so a rejection at any later column leaves those changes behind with no row to show for them — the side effects of a failed addition need to be deferred until every value has validated. And `RowEditor::edit` discards the returned message entirely, then reads a row-ID variable that `addRow` never assigned on the failure path, passing an indeterminate value to the image write and on to the display refresh.

## Copying a Row

Copy is an addition whose defaults come from an existing row rather than from the columns. `PortaBase::copyRow` calls `ViewDisplay::editRow` with the copy flag, which resolves the selected row and hands its ID to `RowEditor::edit`; the dialog pre-fills from `Database::getRow` but, because the copy flag is set, commits through `addRow` rather than `updateRow`. The new row therefore gets a fresh `_id` and freshly allocated sequence values, and the user can amend any field before committing.

**Current state diverges from the documented behavior for image columns.** `ImageSelector` treats its field as unchanged unless the user touches it, and only writes bytes when it is changed, so a copied image field reaches the new row carrying its format string with no image data behind it.

## Deleting Rows

Two paths, both gated on the `confirmDeletions` preference:

- **Single row** — `PortaBase::deleteRow` → `ViewDisplay::deleteRow` (resolving the selected row if no ID was supplied) → `DataModel::deleteRow` → `Database::deleteRow`, which renumbers inline.
- **All rows in the filter** — `PortaBase::deleteAllRows` → `ViewDisplay::deleteAllRows` → `DataModel::deleteAllRows` → `View::deleteAllRows`, which collects the filtered IDs before deleting any of them (so the view cannot shift underneath the loop), deletes each with renumbering suppressed, and compresses once at the end.

`DataModel::deleteAllRows` wraps the bulk path in a full model reset, while the single-row path removes just the affected row from the model.

A bulk delete whose filter matches nothing should do no work. `DataModel::deleteAllRows` returns early on an empty view, but the guard belongs in `View::deleteAllRows` as well, which otherwise runs a full compression pass — a write over every row in the file — after deleting none.

## Display Refresh

`DataModel::addRow` does not assume a new row is visible. It re-prepares the view's data and compares the total row count before and after; if the count did not change, the row failed the current filter and no display update is needed. When it did change, the model locates the new row's index within the view and translates it into the paginated display: a row inserted into an earlier page pushes one row forward, a row inserted into the current page displaces the last row onto the next page, and both cases are signalled as the narrow insert/remove pairs the paginated model expects.

No row operation writes to disk. Each marks the file edited, and the changes reach the file when the user saves.

## Segment Boundary

In scope: the Row menu actions in `PortaBase`, `ViewDisplay`'s row operations, `RowEditor` when it is creating a row (the add and copy branches), `DataModel`'s add and delete refreshes, `View::deleteAllRows`, and `Database`'s `addRow`, `deleteRow`, `compressRowIds`, plus the `maxId` initialization in `load`.

Out of scope: the CSV and XML import paths, which also drive `Database::addRow` but are a distinct user-facing operation belonging to a future segment. Import consumes this segment's contract — a successful `addRow` returns an empty string and yields a unique ID — and that contract is stated here for its consumers rather than owned by them. Editing an existing row in place (`Database::updateRow`) is likewise outside this segment; it shares the `RowEditor` dialog but changes no row's identity. Repairing a damaged file when it is loaded is also outside this segment. That work spans file opening in the application and on the command line, and this segment's only part in it is that `addRow` and `deleteRow` stay safe on any file whose IDs are unique, whether or not they have gaps.

## Decisions & Alternatives

| Decision | Chosen | Alternatives Considered | Rationale |
|----------|--------|--------------------------|-----------|
| Row ID allocation | Monotonic `maxId + 1`, with all higher IDs renumbered down when a row is deleted, keeping IDs dense | Reuse the lowest freed gap, as column IDs do (COL-DB-001); never reuse, allowing permanent gaps | Both keep the ID space bounded, but rows can be renumbered wholesale and columns cannot: a column's ID is embedded in the Metakit property names of every `_data` row, so changing it would rewrite the whole table, while a row's ID is referenced by nothing outside its own row. Renumbering is the cheaper mechanism wherever it is available. [inferred] |
| Where bulk deletes compress IDs | Suppress renumbering per row, compress once after the batch | Renumber inside each `deleteRow` call | Per-row renumbering walks every remaining row for every deletion; one pass after the batch does the same work once. This is the `compressRowIds` flag's only purpose. |
| Bulk delete ordering | Collect every matching ID first, then delete | Delete while iterating the filtered view | The view is derived from the data being deleted, so it shifts under an in-progress loop; snapshotting the IDs removes the dependency entirely. |
| User-facing stable numbering | A Sequence column, allocated from the column's stored default | Expose `_id` as the record number | `_id` renumbers on every deletion, so it cannot carry meaning the user relies on (HLD tenet: *row IDs are internal identity*). A Sequence column's counter only ever moves forward. |
| Row editor commits directly, without staging | `RowEditor` writes through `addRow`/`updateRow` on accept; cancelling simply never calls them | Stage edits and apply on confirm, as the Edit Columns dialog does | The dialog edits exactly one row, so accept is already atomic and cancel already leaves nothing behind. Staging buys the columns editor multi-step atomicity that has no analogue here. [inferred] |
| Image bytes written after the row exists | Two-phase: `addRow` writes the format string, `ImageSelector::saveImage` writes the bytes against the new ID | Pass the image data through `addRow` with the other values | Writing binary field data is addressed by row ID, which does not exist until the row is appended. The format string is an ordinary value and travels the normal path. |
| Copy implemented through the row editor | Pre-fill `RowEditor` from the selected row, commit as a new row | A `Database`-level duplicate-row method | The user can amend the copy before committing, which is the point of the feature; routing through `addRow` also means sequence values are freshly allocated rather than duplicated. |
| Deletion confirmation | Prompt before deleting, gated on the `confirmDeletions` preference | Always prompt; never prompt; undo instead of confirmation | Deletion is not undoable, and the bulk path can remove every row in the filter at once. The preference exists because the prompt is noise to a user doing repeated deletions. [inferred] |
| How `load` initializes `maxId` | The largest `_id` stored in `_data` | The row count minus one; rely on repairing the IDs at load time alone | The row count is correct only when the invariant already holds. Taking the largest stored `_id` costs one line, and it keeps `addRow` from minting a duplicate on any file whose IDs are unique but have gaps, whether or not the file has been repaired. Protecting files with duplicate IDs as well would mean scanning every ID on each add, which is quadratic over a large import, so duplicates are left to the repair. |
| Feedback when an added row fails the current filter | None — the row is stored and the display simply does not change | A transient notification that the row was added but is filtered out | The natural surface for this is a temporary status message, and PortaBase has no such pattern anywhere yet. Introducing one for this case alone would be a new UI convention carried by a single message. |
| Which row is selected after a single-row delete | Deliberately unspecified — whatever the Qt view widget does by default (currently the neighbouring row) | Pin the post-delete selection in a spec and implement it explicitly | The behavior is not important enough to constrain, and specifying it would freeze an assumption about Qt's own default that this project does not control and would have to re-verify on every Qt upgrade. |

## Open Questions & Future Decisions

### Resolved

1. ✅ **Files with non-consecutive row IDs are real, not hypothetical.** The maintainer's `summer_camps.pob` holds 25 rows numbered 74-98. It was the file behind the Edit Columns crash root-caused in the `column-editing` segment, where recalculating a calculated column addressed rows by physical position and missed on the gap (COL-DB-010). That fix makes recalculation survive such a file; it neither prevents the gap from forming nor repairs an existing one. This segment keeps new rows safe on such a file and finds the cause of the gap. Repairing an existing gap is planned separately (Deferred item 5).
2. ✅ **The mechanism that writes non-consecutive IDs is `compressRowIds`, which has written nothing since 2011** (see *Row Identity and the `maxId` Cache*, divergence 3). `ROW-DB-011`'s test reproduces it against current code: after a batch delete and compression, the surviving IDs are unchanged. It explains the known file's shape directly. A "Delete Rows In Filter" that removes the first 74 rows of a 99-row file leaves exactly IDs 74–98. Because `compressRowIds` still sets `maxId` to one less than the row count, rows added afterwards receive IDs counting up from the row count. Those IDs collide with surviving IDs as soon as they reach one: immediately when the deleted rows were scattered, and only after enough additions when, as here, every deleted row was below every survivor. On a later load, the count-derived `maxId` produces the same collisions. Together, the two defects produce duplicates as well as gaps.

### Deferred

1. **EARS for the deletion path are not yet written.** The deletion behavior described above is owned by this segment; only the addition and identity specs have been drafted.
2. **How a copied row should carry its image data across.** That it must is settled — the help text describes copy as pre-filling the editor with the selected row's data, and a row whose format string names a format with no bytes behind it is a state no other path produces — so it is specified as an active gap rather than held open. The implementation choice remains: `ImageSelector` can either load the source row's bytes when its field is set for a copy, or mark the field as needing a write and defer the load until commit.
3. **A copied row's Sequence field displays the source row's number**, not the number that will be stored — `addRow` allocates a fresh one on commit. Cosmetic, but the dialog shows a value it will not write.
4. **The broader row index-versus-ID audit** requested after COL-DB-010: `getId`, `getIndex`, `updateRow`, `editRow`, and the model-to-view index mapping have not been checked for the same confusion between a row's position and its identity.
5. **Repairing a file whose row IDs violate the invariant when it is loaded** is planned as its own segment, since it spans file opening in the application and on the command line rather than any row operation. Its direction is settled. `load` renumbers the rows to 0 through n−1 in memory, ordered by stored `_id` and then by physical position, so no row is dropped. Nothing is saved without the user's consent. In the application, a dialog says that problems were found and fixed and are almost certainly safe, without listing them, and lets the user either save the fixes or close the file unchanged so they can make a backup first. For a read-only file, the dialog says the fixes apply only to this session. On the command line, exports use the repaired data and print a note saying how to make the repair permanent. `fromcsv` asks before saving the repair along with its import, and cancels when it has no terminal to ask on. Format upgrades in `load` stay silent, because they transform the file rather than correct damage. This segment's specs do not depend on the repair: ROW-DB-002 and ROW-DB-010 keep `addRow` and `deleteRow` safe without it on files with unique IDs. Files with duplicate IDs need the repair.

## References

- Arrow doc: `docs/arrows/row-lifecycle.md`
- Help documentation: `resources/help/row_editor.txt`, `resources/help/data_viewer.txt` (Row menu)
- [PortaBase format specification](../../wiki/format.md) — the `_data` view and its `_id` invariant
- Sibling segments: `docs/intent/column-editing/column-editing-design.md` (COL-DB-010, the calculated-column recalculation that non-consecutive IDs crashed), `docs/intent/integrity-check/integrity-check-design.md` (CHK-DATA-003, which reports the violated invariant)
- Code: `src/portabase.cpp` (`deleteRow`, `copyRow`, `deleteAllRows`); `src/viewdisplay.h`, `src/viewdisplay.cpp` (`addRow`, `editRow`, `deleteRow`, `deleteAllRows`); `src/roweditor.h`, `src/roweditor.cpp` (`edit`, `addContent`, `getRow`, `isValid`); `src/datamodel.h`, `src/datamodel.cpp` (`addRow`, `deleteRow`, `deleteAllRows`); `src/view.cpp` (`deleteAllRows`); `src/database.h`, `src/database.cpp` (`addRow`, `deleteRow`, `compressRowIds`, and `load`'s `maxId` initialization)
- Known lower-priority findings on this code: `docs/wiki/dbeditor-code-review-followups.md`
