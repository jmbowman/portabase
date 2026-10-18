# High-Level Design

## Problem

*(not yet specified — scope is too narrow for a project-level problem statement; only a few segments have been mapped so far. See `docs/wiki/roadmap.md` and `docs/wiki/competitive-landscape.md` for the existing, non-LID-formatted project context.)*

## Approach

*(not yet specified)*

## Target Users

*(not yet specified)*

## Goals

*(not yet specified)*

## Non-Goals

*(not yet specified)*

## Tenets

- **Never trust file-derived state.** A `.pob` file can violate even the invariants PortaBase itself maintains; files accumulate decades of history across versions, platforms, and past bugs.
- **Diagnose before repair.** Tools that examine user data report findings; they never mutate a file except by an explicit, separate user action.
- **Diagnostic scans stay cheap on clean files.** A checker's normal outcome is "no findings", so its cost should track what is actually wrong rather than the size of the database.
- **Row IDs are internal identity, not a user-visible record number.** A row's `_id` addresses it within the file, so operations are free to renumber rows to keep the ID space compact.

*(Additional tenets to be elicited as more segments are mapped. See `docs/intent/column-editing/column-editing-design.md`'s Decisions & Alternatives for segment-level design principles, e.g. staged edits with atomic apply-on-confirm.)*

## System Design

*(not yet specified at whole-project level — three segments are mapped so far; see `docs/arrows/index.yaml` for the overlay.)*

- `column-editing` (`docs/intent/column-editing/`) — the Edit Columns operation end-to-end: the staging dialog, the `Database` schema-mutation methods it drives, and the post-accept view maintenance and refresh.
- `integrity-check` (`docs/intent/integrity-check/`) — read-only structural validation of `.pob` files against the file-format invariants (the [PortaBase format specification](wiki/format.md)), surfaced as a `check` subcommand of the PortaBase command-line interface.
- `row-lifecycle` (`docs/intent/row-lifecycle/`) — the lifecycle of a data row: adding one (including copy-as-new), deleting one or many, and the `_id` assignment both depend on. Covers the row editor dialog, the `Database` row-mutation methods, and the data-viewer refresh. The CSV/XML import paths also drive `Database::addRow`, but as a separate operation outside this segment.

## Key Design Decisions

- **Invariant maintenance during user-initiated operations.** In the course of an operation the user started, the application may auto-repair a format anomaly when the repair path is unambiguous and loses no data — for example, the Edit Columns operation reconciling the `_all` view (COL-DB-008). This is invariant maintenance, not repair tooling, so *Diagnose before repair* does not apply to it.

## Success Metrics

*(not yet specified.)*

## References

- `docs/wiki/roadmap.md`, `docs/wiki/agentic_sdlc.md`, `docs/wiki/competitive-landscape.md` — existing project-level context, not yet folded into this LID tree.
- [PortaBase format specification](wiki/format.md) — the authoritative `.pob` file-format definition (Metakit view layouts, invariants, version history).
- `docs/arrows/index.yaml` — the arrow overlay index for the mapped segments.
