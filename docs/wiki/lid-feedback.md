# LID Feedback

Issues and inconsistencies noticed while piloting LID (`jszmajda/lid`, installed at `v1.3.0` via APM — see [agentic-development-support.md](agentic-development-support.md)) against PortaBase's `dbeditor`/`database` schema-editing code. Intended to be passed upstream to the LID maintainer.

## Open Items

### 1. Ambiguous Whether `map-codebase` Adds `@spec` Annotations to Source Code

- **Where:** `plugins/arrow-maintenance/skills/map-codebase/SKILL.md` vs. `plugins/arrow-maintenance/skills/map-codebase/references/brownfield-bootstrap.md`
- **What:** The canonical `SKILL.md` Phase 5 ("Artifact Generation") lists exactly four output types, all new files under `docs/`: per-segment arrow doc, skeleton LLD, EARS spec file, `index.yaml` entry. It never mentions touching source files.
- **But:** `brownfield-bootstrap.md` — which its own header says maps "older section numbering" onto the current phase structure, implying it may not be fully updated — has a "Step 5b: Add `@spec` annotations to code... add `// @spec ID` comments to the implementing code and test files," directly contradicting the canonical list.
- **Impact:** A user relying on `SKILL.md` alone (reasonably, since it's marked canonical) would not expect source files to be modified during brownfield mapping; the reference doc suggests otherwise. This matters a lot for a pilot/trial run on a codebase with no VCS safety net assumptions, or just for setting correct expectations.
- **Suggested fix:** Either update `brownfield-bootstrap.md` to drop the stale step, or make `SKILL.md`'s Phase 5 list explicitly complete/authoritative with a note that the reference doc's older step 5b is superseded.

### 2. LLD File Path Convention Disagrees Between Two Docs

- **Where:** `plugins/arrow-maintenance/skills/map-codebase/SKILL.md` Phase 5, item 2, vs. `plugins/linked-intent-dev/skills/linked-intent-dev/references/lld-templates.md`
- **What:** `SKILL.md` says: "Skeleton LLD at the mirroring path `docs/intent/{segment-path}.md`" — implying a flat file.
- **But:** `lld-templates.md` (which `SKILL.md` itself cites as "the standard LLD template") is explicit and detailed: "Every node is a **directory** under `/docs/intent/`... A leaf LLD `foo` is `docs/intent/foo/foo-design.md`, with its EARS beside it as `foo-specs.md`" — a directory-per-node convention, not a flat file.
- **Also inconsistent with:** `SKILL.md` Phase 5, item 3 itself, which says the EARS spec file goes "beside the segment's design doc" inside `docs/intent/<segment-path>/` — which only makes sense if the design doc is *inside* that same directory, not a sibling flat file one level up.
- **Resolution used for this pilot:** Trusted `lld-templates.md`'s directory convention (`docs/intent/column-editing/column-editing-design.md`) since it's the more detailed, explicit, and internally consistent source, and because it's needed anyway to support sub-HLD promotion (nodes must be directories to hold children later).

- **Suggested fix:** Update `SKILL.md` Phase 5 item 2's wording to match `lld-templates.md`'s directory convention.

### 3. `index.yaml` Example in `brownfield-bootstrap.md` Uses a Stale Schema Version

- **Where:** `plugins/arrow-maintenance/skills/map-codebase/references/brownfield-bootstrap.md` Phase 5d example, vs. `plugins/arrow-maintenance/skills/arrow-maintenance/references/index-schema.md`
- **What:** The worked example in `brownfield-bootstrap.md` shows `schema_version: 1` with a `detail` field description that doesn't match. The authoritative `index-schema.md` is at `schema_version: 2`, which added `parent`/`children` design-tree links and redefined `detail` as "the doc to open on this node." Used v2 (the authoritative current schema) for this pilot.
- **Impact:** Minor — same root cause as issue #2, but flagging since it's the third instance of the same pattern (a detailed/authoritative reference doc having moved on from an older walkthrough example that wasn't updated to match).
- **Suggested fix:** Given this is the third instance of stale-example drift, might be worth a general pass checking whether `brownfield-bootstrap.md`'s worked examples are all still current, rather than fixing these one at a time.

## Not Yet Logged as Issues, but Worth the Maintainer Double-Checking

- Nothing else so far — this pilot is still in progress (arrow doc, LLD, EARS specs, and `index.yaml` written for one segment; HLD is the last artifact).
