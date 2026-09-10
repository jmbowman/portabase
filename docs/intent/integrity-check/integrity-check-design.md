---
parent: high-level-design
prefix: CHK
---

# Data Integrity Check

## Context and Design Philosophy

PortaBase files accumulate decades of history: schema edits across many application versions, platforms, and past bugs. The application's own code assumes the file's internal structures are consistent — unique column IDs, view references that resolve, contiguous position sequences — but nothing ever verifies those assumptions, and much of the reading code does not guard against their failure (see `docs/intent/column-editing/column-editing-design.md` § Robustness to inconsistent file state). A latent inconsistency can therefore ride along silently for years and only surface as a crash when some operation finally trips over it.

This component makes those assumptions checkable: a read-only scan of a `.pob` file against the invariants of the file format (the [PortaBase format specification](../../wiki/format.md)), reporting every violation found. It follows both HLD tenets directly — it *never trusts file-derived state* (every lookup handles the miss case; that is the point of the tool), and it *diagnoses before repair* (it reports; it never writes).

The immediate use is scanning real, long-lived files for latent corruption — the kind of inconsistency that rides along silently until some operation trips over it. A scan did exactly that once: it surfaced the non-consecutive row IDs behind the originally-reported add-column crash (`COL-DB-010`). The catalog is general, though: it verifies what the format specification and the application code jointly assume, not just crash candidates.

## Architecture

Two pieces, deliberately separated:

- **Checker core** — `src/integritycheck.{h,cpp}`, a QtCore-only class (no widgets, no dependency on `Database`). Opens the file via Metakit directly, runs the invariant catalog, returns a list of findings. Reusable by any surface without change.
- **CLI surface** — a `check` subcommand of PortaBase's existing command-line interface (`src/commandline.cpp`, dispatched from `main.cpp` like the `fromxml`/`tocsv`/… commands): parse the file arguments, run the core on each, print a human-readable report per file, and return a combined exit code.

Delivering the checker as a subcommand of the one `portabase` binary (rather than a standalone tool) means it ships with every install and is discoverable in the same `--help`, at no extra build or packaging cost. The checker core stays decoupled from `Database` and widgets, so the subcommand cannot crash the way the application can, and the same core remains reusable by a future in-app dialog or open-time validation hook.

### File access discipline

- The Metakit storage is opened **read-only** (`c4_Storage` with the read-write flag off). The checker must be safe to point at a user's only copy of a file.
- Structures are read through **raw Metakit views obtained by name**, never through `Database`'s accessor methods. `Database` is the code whose unguarded assumptions the checker exists to test; reusing it would import the very crashes being diagnosed, and its constructor upgrades/mutates old files as a side effect.
- Every property read is preceded by a presence check (view exists, row exists, property present in the view's format). A structurally alien file (not Metakit, truncated, missing `_global` entirely, or with no readable `_gversion`) produces an "unreadable / not a PortaBase file" result, not a crash.

### Finding model

Each finding carries:

| Field | Meaning |
|-------|---------|
| check ID | The EARS spec ID of the violated invariant (`CHK-…`) |
| severity | `ERROR` or `WARNING` (see below) |
| location | Which view/row/column/name the violation was found at |
| message | Human-readable description with the offending values |

**Severity principle:** `ERROR` for violations of assumptions the application relies on without checking (crash or data-loss risk — e.g. a `_viewcolumns` row naming a nonexistent column); `WARNING` for inconsistencies the application tolerates or that only degrade behavior (e.g. residual `_data` properties left by a deleted column, or an enum option-index mismatch that mis-sorts but cannot crash). Severity is classified against the **released-app population**, not just the current codebase: a violation stays `ERROR` while any extant PortaBase release relies on it unguarded, even after the development tree gains guards — the checker's purpose is triaging real files used with real releases.

**Aggregation:** per-row checks over `_data` (which may have tens of thousands of rows) report the first few instances individually, then a count — a corrupt enum column must not produce ten thousand findings.

**Anomaly detail:** where a violation's whole point is diagnosing how a file reached its present state — `_id` values above all, since they carry the history of what was added and removed — the finding describes what the data *is*, not only how it differs from the expectation. A file whose IDs are entirely outside the expected range is otherwise reported as a list of everything missing, which conveys nothing about the block that is actually there. Contiguous runs collapse to a range, so an intact-but-offset sequence is legible at a glance.

**Scan cost:** per the HLD tenet *diagnostic scans stay cheap on clean files*, a check that scans every row holds no per-row state while the file is clean — it accumulates only a fixed-size verdict, and gathers the detail a finding needs (offending values, counts, extremes) in a second pass made only once the violation is confirmed. Re-reading a view is acceptable on that path; the file is already known to be broken, and the alternative is charging every clean file for a report it will never produce.

**Check dependencies:** checks with no prerequisites always run. A check whose premise is invalidated by another check's failure (e.g. the `_all`-ordering check when `_cindex` is not a permutation) is skipped, with an explicit note naming the failed prerequisite check — a skipped check must never be silently indistinguishable from a passed one.

### Version and encryption gating

- Each check is gated on the **format version** (`_gversion`) at which its subject became part of the format, per the [PortaBase format specification](../../wiki/format.md)'s version history (e.g. `_cid` uniqueness only applies at version ≥ 4, `_calcs`/`_calcnodes` checks at version ≥ 10). A file is checked against the catalog appropriate to its own declared version; the declared version itself is checked against the range this build understands — 1 through the newest version it writes (`FILE_VERSION`).
- **Encrypted files** (`_gcrypt` = 1): version 1 of the checker validates only the outer structure (exactly the `_global` and `_crypto` views present, `_crypto` holding one row with the expected properties) and reports that the encrypted contents were not checked. Decryption requires the password and re-implements the crypto path; deferred.

## Invariant Catalog

Grouped by subject; each line becomes one or more EARS specs. "References" always means by-name (or by-ID) resolution against the referenced view.

**Structure**: all mandatory views for the file's format version are present (`_global`, `_columns`, `_data`, `_views`, `_viewcolumns` always; `_sorts`/`_sortcolumns` at version ≥ 2; `_filters`/`_filterconditions` at version ≥ 3; `_enums`/`_enumoptions` at version ≥ 4; `_calcs`/`_calcnodes` at version ≥ 10); every stored string property across all views is valid UTF-8 (WARNING — display corruption, not crash).

**`_global`**: exactly one row; `_gversion` in the known range; `_gview` references an existing view; `_gsort` empty or references an existing sorting; `_gfilter` empty or references an existing filter; `_gcrypt` consistent with the presence/absence of `_crypto`.

**`_columns`**: `_cid` unique; `_cname` unique, non-empty, not starting with `_`; `_cindex` a permutation of 0..n−1; `_ctype` a valid type code (0–9, or ≥ 100 with a matching `_enums` row); `_cdefault` parseable for the column's type (WARNING).

**`_data`**: the stored format string (Metakit `Description`) matches the format derived from `_columns`, extra residual properties being a WARNING; `_id` values unique and consecutive from 0; for enum columns, each row's option text exists in the enum's options and the stored option index matches that option's `_eoindex` (WARNING — this is the exact corruption format version 9 existed to flush out).

**`_views` / `_viewcolumns`**: a view named `_all` exists; `_all`'s column set is exactly the `_columns` name set, ordered by `_vcindex` to match `_cindex` order; `_vname` unique; every `_vcview` references an existing view (orphan detection); every `_vcname` references an existing column; per view, `_vcindex` a permutation of 0..k−1; `_vsort`/`_vfilter` are `_none` or reference an existing sorting/filter.

**`_sorts` / `_sortcolumns`**: `_sname` unique; every `_scsort` references an existing sorting; every `_scname` references an existing column; per sorting, `_scindex` a permutation of 0..k−1; `_scdesc` is 0 or 1.

**`_filters` / `_filterconditions`**: a filter named `_allrows` exists; `_fname` unique; every `_fcfilter` references an existing filter; every `_fccolumn` references an existing column or is `_anytext`; per filter, `_fcposition` a permutation of 0..k−1; `_fcoperator` in 0–7; `_fccase` 0 or 1.

**`_enums` / `_enumoptions`**: `_ename` unique; `_eid` unique and ≥ 100; `_eindex` a permutation of 0..n−1; every `_eoenum` references an existing `_eid`; per enum, `_eoindex` a permutation of 0..k−1.

**`_calcs` / `_calcnodes`**: every column of type CALC has exactly one `_calcs` row keyed by its `_cid`; every `_calcs` row references an existing CALC-typed column; every `_cnid` references an existing calculation; column-reference nodes (`_cntype` 1, 3, 5) name existing columns; node type codes are in the documented set.

## Testing

Checker tests live in `tests/integritycheck/` (QtTest), one fixture-corruption scenario per invariant: build a structurally valid file, seed exactly one violation, assert the expected finding (check ID, severity, location). The valid-fixture builder and raw-Metakit corruption seeder live in `tests/common/`, shared with the column-editing segment as test infrastructure owned by neither segment. Encrypted-file outer-structure checks are tested by seeding a synthetic `_crypto` view and `_gcrypt` flag — no real encryption needed, since only the outer structure is validated (CHK-CORE-004).

## CLI Behavior

`portabase check <file> [<file>…]` — checks each named file and prints a per-file report: the filename, format version, each finding (severity, check ID, location, message), and a one-line summary. A clean file prints its summary line only. The `check` command is dispatched by `CommandLine` alongside the existing import/export commands, and appears in `portabase --help`.

Exit codes: `0` — all files clean; `1` — at least one finding in at least one file; `2` — at least one file could not be opened or is not a PortaBase file. (`2` dominates `1`.)

## Decisions & Alternatives

| Decision | Chosen | Alternatives Considered | Rationale |
|----------|--------|--------------------------|-----------|
| Delivery surface | A `check` subcommand of PortaBase's existing CLI, over a reusable QtCore checker core | Standalone `pbcheck` binary; in-app dialog; validation inside `Database::open` | The subcommand ships with every install and is discoverable in the same `--help`, at no extra build/packaging cost — and PortaBase's CLI already runs under `QApplication`, so a standalone QtCore tool bought no headless advantage the CLI didn't already have. The core stays decoupled from `Database`/widgets, so the subcommand still cannot crash the way the app does and remains reusable by a future in-app surface. |
| File access | Raw Metakit views, storage opened read-only | Reuse `Database`'s accessors | `Database` is the code under suspicion — its accessors carry the unguarded lookups being diagnosed, and its constructor mutates old-format files on open. The checker must be safe on a user's only copy. |
| Check vs. repair | Report only; no mutation ever | Offer auto-repair for mechanical fixes (e.g. renumbering) | HLD tenet: diagnose before repair. Repair needs its own design once real-world findings show what actually occurs. |
| Version handling | Gate each check on the format version that introduced its subject | Support only current-format (v11) files; upgrade-then-check | The point is scanning old, long-lived files as they are; upgrading first would both mutate the file and mask version-specific corruption. |
| Float/calc string-vs-value consistency in `_data` | Not checked | Compare the stored string column against the stored float | The string is written with the *writing* machine's locale; re-parsing on the checking machine produces false positives. Revisit if a locale-safe comparison is found. |
| Permutation checking | A single non-mutating pass marking a visited bitset sized to the value count | Sort a copy and compare against 0..n-1; Metakit's `SortOn` over the view | Sorting mutates, so it deep-copies its argument at every call site under Qt's implicit sharing (see `docs/wiki/qt-tips.md`) — and four of the call sites pass a value held in a `QMap`, where no in-place option exists. `SortOn` is worse still: its `c4_SortSeq` keeps two `c4_DWordArray` permutation maps (8 bytes/row) plus a projection sequence, more memory than the IDs being checked, and it introduces derived-view machinery over data already suspected of corruption. The bitset is sized by the number of values, never by a corrupt file's largest one. |
| Shared format constants | Include `src/datatypes.h` for the column type codes and `FILE_VERSION` | Redeclare them locally in the checker's anonymous namespace | A second copy of the type codes is a second source of truth for wire-format values, free to drift. `datatypes.h` is dependency-light (only `<QList>`), so including it does not compromise the QtCore-only decoupling above — unlike `database.h`, which pulls in `<QPixmap>` and `<mk4.h>`. |
| Output format | Human-readable text | JSON/machine-readable | The consumer today is a human (and an agent reading terminal output). A `--json` flag is cheap to add later without redesign. |

## Open Questions & Future Decisions

### Deferred

1. **Checking encrypted file contents** — requires the password and the Blowfish/CBC decryption path; v1 validates outer structure only.
2. **Repair mode** — deliberately out of scope until real-world scan results show which violations actually occur.
3. **Deep `_calcnodes` tree validation** (single root, acyclic, parent IDs resolve) — v1 checks references only; full tree-shape validation deferred until calc corruption is actually observed.
4. **In-app surfaces** (menu dialog, open-time validation) — add if scans show corruption is prevalent in real files. The checker core is already decoupled to make this a pure addition.
5. **`-p password` for encrypted contents** — the `check` subcommand lives in `CommandLine`, which already parses `-p`; wiring it to decrypt and run the full catalog (CHK-CORE-009) is a natural follow-on.

## References

- [PortaBase format specification](../../wiki/format.md) (the invariant source of truth)
- Column-editing crash it helped diagnose: `docs/intent/column-editing/column-editing-design.md` § Robustness to inconsistent file state; specs `COL-DB-007`/`COL-DB-010`
- Arrow doc: `docs/arrows/integrity-check.md`
- Vendored Metakit API: `metakit/include/mk4.h` (`c4_Storage::Description`, read-only open)
