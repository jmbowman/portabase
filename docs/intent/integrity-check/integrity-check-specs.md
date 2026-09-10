# Data Integrity Check — EARS Specs

Severities cited below follow the classification principle in `integrity-check-design.md` § Finding model: ERROR for violations of assumptions the application relies on without checking (crash or data-loss risk), WARNING for inconsistencies the application tolerates or that only degrade behavior.

## CHK-CORE (checker engine)

- [x] **CHK-CORE-001**: The integrity checker shall never write to a file being checked, opening its Metakit storage read-only.
- [x] **CHK-CORE-002**: If a file cannot be parsed as Metakit, has no `_global` view, or has no readable `_gversion` value, then the checker shall report the file as unreadable / not a PortaBase file, without crashing and without running the invariant catalog against it.
- [x] **CHK-CORE-003**: The checker shall run each invariant check only against files whose declared format version (`_gversion`) is one where the check's subject exists, per the format version history in the [PortaBase format specification](../../wiki/format.md).
- [x] **CHK-CORE-004**: When checking an encrypted file (`_gcrypt` = 1), the checker shall validate only the outer structure (exactly the `_global` and `_crypto` views present; `_crypto` holding one row with `_criv`, `_crhash`, and `_crdata`) and shall report that the encrypted contents were not checked.
- [x] **CHK-CORE-005**: If a check's prerequisite check has failed (e.g. the `_all`-ordering check when `_cindex` is not a permutation), then the checker shall skip the dependent check and report the skip with the failed prerequisite's check ID.
- [x] **CHK-CORE-006**: When a per-row check over `_data` fails on many rows, the checker shall report the first three violating rows individually followed by a count of the remainder, per check per column.
- [x] **CHK-CORE-007**: The checker shall report a WARNING for any stored string property value, in any view, that is not valid UTF-8.
- [x] **CHK-CORE-008**: The checker shall report an ERROR for each mandatory view absent for the file's format version (`_global`, `_columns`, `_data`, `_views`, `_viewcolumns` always; `_sorts` and `_sortcolumns` at version ≥ 2; `_filters` and `_filterconditions` at version ≥ 3; `_enums` and `_enumoptions` at version ≥ 4; `_calcs` and `_calcnodes` at version ≥ 10), except in encrypted files (see CHK-CORE-004).
- [D] **CHK-CORE-009**: Where the user supplies the file's password, the checker shall decrypt an encrypted file's contents and run the full invariant catalog against them.

## CHK-GLOBAL (the `_global` view)

- [x] **CHK-GLOBAL-001**: The checker shall report an ERROR if `_global` does not contain exactly one row.
- [x] **CHK-GLOBAL-002**: The checker shall report an ERROR if `_gversion` is outside the range of format versions this build understands — 1 through the newest version this build of PortaBase writes (currently 11). A file declaring a newer version was written by a newer PortaBase, which this build's application likewise refuses to open.
- [x] **CHK-GLOBAL-003**: The checker shall report an ERROR if `_gview` does not name an existing view in `_views`.
- [x] **CHK-GLOBAL-004**: The checker shall report an ERROR if `_gsort` is neither empty nor the name of an existing sorting in `_sorts`.
- [x] **CHK-GLOBAL-005**: The checker shall report an ERROR if `_gfilter` is neither empty nor the name of an existing filter in `_filters`.
- [x] **CHK-GLOBAL-006**: The checker shall report an ERROR if `_gcrypt` = 1 in a file with no `_crypto` view, or if a `_crypto` view is present in a file whose `_gcrypt` is 0.

## CHK-COL (the `_columns` view)

- [x] **CHK-COL-001**: The checker shall report an ERROR for each `_cid` value shared by more than one `_columns` row.
- [x] **CHK-COL-002**: The checker shall report an ERROR for each `_cname` value that is duplicated, empty, or begins with an underscore.
- [x] **CHK-COL-003**: The checker shall report a WARNING if the `_cindex` values of `_columns` are not a permutation of 0 through n−1.
- [x] **CHK-COL-004**: The checker shall report an ERROR for each `_ctype` value that is not a documented type code (0–9) and not an enum type with a matching `_eid` in `_enums`.
- [x] **CHK-COL-005**: The checker shall report a WARNING for each `_cdefault` value that cannot be parsed as its column's declared type, for integer, sequence, boolean, and enum columns (other types' defaults are stored in locale-dependent or sentinel-encoded forms that cannot be validated portably).

## CHK-DATA (the `_data` view)

- [x] **CHK-DATA-001**: The checker shall report an ERROR for each data property required by the column set (per the format's column-ID derivation rules) that is missing from the stored `_data` format.
- [x] **CHK-DATA-002**: The checker shall report a WARNING for each property present in the stored `_data` format that is not `_id` and is not derivable from any current column (residue of a deleted column).
- [x] **CHK-DATA-003**: The checker shall report an ERROR if the `_id` values of `_data` are not unique and consecutive from 0 through rowCount−1, describing the IDs actually present as well as which are duplicated, which of the expected values are missing, and the largest present. Consecutive runs of present IDs shall be reported as a range rather than value by value.
- [x] **CHK-DATA-004**: The checker shall report a WARNING for each enum-typed data value whose option text is not among its enum's options in `_enumoptions`.
- [x] **CHK-DATA-005**: The checker shall report a WARNING for each enum-typed data value whose stored option index does not equal the `_eoindex` of the option matching its stored text.

## CHK-VIEW (the `_views` and `_viewcolumns` views)

- [x] **CHK-VIEW-001**: The checker shall report an ERROR if no view named `_all` exists in `_views`.
- [x] **CHK-VIEW-002**: The checker shall report an ERROR if the `_all` view's column entries are not exactly the `_columns` name set, ordered by `_vcindex` to match the `_cindex` order.
- [x] **CHK-VIEW-003**: The checker shall report a WARNING for each `_vname` value shared by more than one `_views` row.
- [x] **CHK-VIEW-004**: The checker shall report a WARNING for each `_viewcolumns` row whose `_vcview` does not name an existing view (orphaned view-column entry).
- [x] **CHK-VIEW-005**: The checker shall report an ERROR for each `_viewcolumns` row whose `_vcname` does not name an existing column.
- [x] **CHK-VIEW-006**: The checker shall report a WARNING for each view whose `_vcindex` values are not a permutation of 0 through k−1.
- [x] **CHK-VIEW-007**: The checker shall report an ERROR for each view whose `_vsort` is neither `_none` nor an existing sorting, or whose `_vfilter` is neither `_none` nor an existing filter.

## CHK-SORT (the `_sorts` and `_sortcolumns` views)

- [x] **CHK-SORT-001**: The checker shall report a WARNING for each `_sname` value shared by more than one `_sorts` row.
- [x] **CHK-SORT-002**: The checker shall report a WARNING for each `_sortcolumns` row whose `_scsort` does not name an existing sorting (orphaned sorting-column entry).
- [x] **CHK-SORT-003**: The checker shall report an ERROR for each `_sortcolumns` row whose `_scname` does not name an existing column.
- [x] **CHK-SORT-004**: The checker shall report a WARNING for each sorting whose `_scindex` values are not a permutation of 0 through k−1, or containing an `_scdesc` value other than 0 or 1.

## CHK-FILTER (the `_filters` and `_filterconditions` views)

- [x] **CHK-FILTER-001**: The checker shall report a WARNING if no filter named `_allrows` exists in `_filters`.
- [x] **CHK-FILTER-002**: The checker shall report a WARNING for each `_fname` value shared by more than one `_filters` row.
- [x] **CHK-FILTER-003**: The checker shall report a WARNING for each `_filterconditions` row whose `_fcfilter` does not name an existing filter (orphaned condition).
- [x] **CHK-FILTER-004**: The checker shall report an ERROR for each `_filterconditions` row whose `_fccolumn` neither names an existing column nor is `_anytext`.
- [x] **CHK-FILTER-005**: The checker shall report a WARNING for each filter whose `_fcposition` values are not a permutation of 0 through k−1, containing an `_fcoperator` outside 0–7, or containing an `_fccase` value other than 0 or 1.

## CHK-ENUM (the `_enums` and `_enumoptions` views)

- [x] **CHK-ENUM-001**: The checker shall report an ERROR for each `_ename` value shared by more than one `_enums` row, and for each `_eid` value that is duplicated or less than 100.
- [x] **CHK-ENUM-002**: The checker shall report a WARNING if the `_eindex` values of `_enums` are not a permutation of 0 through n−1.
- [x] **CHK-ENUM-003**: The checker shall report a WARNING for each `_enumoptions` row whose `_eoenum` does not match an existing `_eid` (orphaned option).
- [x] **CHK-ENUM-004**: The checker shall report a WARNING for each enum whose `_eoindex` values are not a permutation of 0 through k−1.

## CHK-CALC (the `_calcs` and `_calcnodes` views)

- [x] **CHK-CALC-001**: The checker shall report a WARNING for each column of type CALC (7) that does not have exactly one `_calcs` row keyed by its `_cid`.
- [x] **CHK-CALC-002**: The checker shall report a WARNING for each `_calcs` row whose `_calcid` does not match the `_cid` of an existing CALC-typed column (orphaned calculation).
- [x] **CHK-CALC-003**: The checker shall report a WARNING for each `_calcnodes` row whose `_cnid` does not match an existing calculation (orphaned node).
- [x] **CHK-CALC-004**: The checker shall report an ERROR for each column-reference calculation node (`_cntype` 1, 3, or 5) whose `_cnvalue` does not name an existing column.
- [x] **CHK-CALC-005**: The checker shall report a WARNING for each `_calcnodes` row whose `_cntype` is not a documented node type code.

## CHK-CLI (the `check` subcommand of the PortaBase CLI)

- [x] **CHK-CLI-001**: The PortaBase command-line interface shall accept a `check` command taking one or more file-path arguments, and check each named file.
- [x] **CHK-CLI-002**: For each checked file, the `check` command shall print the file name, its format version, each finding (severity, check ID, location, message), each skipped check with its failed prerequisite, and a one-line summary of finding counts by severity.
- [x] **CHK-CLI-003**: For a file with no findings, the `check` command shall print only the file name, format version, and clean summary line.
- [x] **CHK-CLI-004**: The PortaBase process, when invoked with `check`, shall exit with code 0 when all checked files are clean, 1 when any file produced findings, and 2 when any file was unreadable or not a PortaBase file; unreadable (2) takes precedence over findings (1).
