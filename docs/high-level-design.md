# High-Level Design

## Problem

*(not yet specified — scope is too narrow for a project-level problem statement; only one segment has been mapped so far. See `docs/wiki/roadmap.md` and `docs/wiki/competitive-landscape.md` for the existing, non-LID-formatted project context.)*

## Approach

*(not yet specified)*

## Target Users

*(not yet specified)*

## Goals

*(not yet specified)*

## Non-Goals

*(not yet specified)*

## Tenets

- **Never trust file-derived state.** Code reading a `.pob` file's internal structures must handle the lookup-miss and invariant-violation cases, even for invariants PortaBase itself is supposed to maintain — files accumulate decades of history across versions, platforms, and past bugs.
- **Diagnose before repair.** Tools that examine user data report findings; they never mutate a file except by an explicit, separate user action. The application itself, in the course of a user-initiated operation, may auto-repair a format anomaly when the repair path is unambiguous and loses no data — that is invariant maintenance, not repair tooling.

- **Diagnostic scans stay cheap on clean files.** A checker's normal outcome is "no findings", so its per-record cost should track what is actually wrong rather than the size of the database — build the detail needed to describe a violation only once one is confirmed.

*(Additional tenets to be elicited as more segments are mapped. See `docs/intent/column-editing/column-editing-design.md`'s Decisions & Alternatives for segment-level design principles, e.g. staged edits with atomic apply-on-confirm.)*

## System Design

*(not yet specified at whole-project level — two segments are mapped so far; see `docs/arrows/index.yaml` for the overlay.)*

- `column-editing` (`docs/intent/column-editing/`) — the Edit Columns operation end-to-end: the staging dialog, the `Database` schema-mutation methods it drives, and the post-accept view maintenance and refresh.
- `integrity-check` (`docs/intent/integrity-check/`) — read-only structural validation of `.pob` files against the file-format invariants (the [PortaBase format specification](wiki/format.md)), surfaced as a `check` subcommand of the PortaBase command-line interface.

## Key Design Decisions

*(not yet specified at project level.)*

## Success Metrics

*(not yet specified.)*

## References

- `docs/wiki/roadmap.md`, `docs/wiki/agentic_sdlc.md`, `docs/wiki/competitive-landscape.md` — existing project-level context, not yet folded into this LID tree.
- [PortaBase format specification](wiki/format.md) — the authoritative `.pob` file-format definition (Metakit view layouts, invariants, version history).
- `docs/arrows/index.yaml` — the arrow overlay index for the mapped segments.
