# Arrow: integrity-check

Read-only structural validation of `.pob` files against the file-format invariants (the [PortaBase format specification](../wiki/format.md)), surfaced as a `check` subcommand of the PortaBase CLI over a reusable QtCore checker core.

## Status

**OK** — greenfield segment designed 2026-07-13; checker core implemented tests-first 2026-07-14; delivered as the `portabase check` subcommand 2026-07-15; audited 2026-07-18 with all CHK specs implemented and tested. Only deferral: `CHK-CORE-009` (encrypted-content checking).

## References

### HLD
- docs/high-level-design.md

### LLD
- docs/intent/integrity-check/integrity-check-design.md

### EARS
- docs/intent/integrity-check/integrity-check-specs.md

### Tests
- tests/integritycheck/integritycheck_test.cpp (QtTest — table-driven, one corruption scenario per catalog invariant, engine/report/exit-code tests, plus `cliCheckCommand` which drives the built `portabase check` binary end-to-end via QProcess (`CHK-CLI-001`), skipping if the app is not built; all passing)
- tests/common/ — shared fixture builder and raw-Metakit corruption seeder

### Code
- src/integritycheck.h, src/integritycheck.cpp (checker core; linked into the app via common.pri)
- src/commandline.cpp (`checkFiles` — the `check` subcommand), src/main.cpp (CLI dispatch of `check`)

## Architecture

**Purpose:** Scan an existing PortaBase file for violations of the invariants the application's code assumes but never verifies — missing/duplicate IDs, broken position sequences, orphaned view/sort/filter/calc references, schema vs. format-string mismatches — and report them without ever writing to the file.

**Key Components:**
1. Checker core (`src/integritycheck.{h,cpp}`) — QtCore-only; opens the file read-only via raw Metakit views (never through `Database`), runs the version-gated invariant catalog, returns findings (check ID = EARS spec ID, severity, location, message).
2. `check` subcommand (`CommandLine::checkFiles`, dispatched from `main.cpp`) — thin wrapper; per-file human-readable report; exit codes 0 clean / 1 findings / 2 unreadable.

## Spec Coverage

- **CHK-CORE** (checker engine): 001–008 implemented; 009 (encrypted-content checking) deferred.
- **CHK-GLOBAL / CHK-COL / CHK-DATA / CHK-VIEW / CHK-SORT / CHK-FILTER / CHK-ENUM / CHK-CALC** (invariant catalog): all implemented.
- **CHK-CLI** (the `check` subcommand): 001–004 implemented and tested — 001 (the argument loop) by `cliCheckCommand` driving the built binary, the rest via the core's `report()`/`exitCode()`.

## Work Required

### Must Fix
(none — implementation matches the specs)

### Should Fix
(none yet)

### Nice to Have
(none)
