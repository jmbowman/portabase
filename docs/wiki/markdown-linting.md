# Markdown Linting

This project lints `docs/` with two complementary tools. `rumdl` owns all general-purpose Markdown rules; `mdbook-lint` is kept only for the book-structure checks nothing else performs. Neither replaces the other.

For the conversion pipeline that feeds ported pages into this tree, see `docs/wiki/converting-html-to-markdown.md`.

## Why Two Linters

`mdbook-lint` is the only tool that understands an mdBook as a book — that `SUMMARY.md` is a table of contents, that a file missing from it silently never renders, that a chapter title should match its page's H1. Nothing else checks any of that.

Its general-purpose rules are weaker, though, and they misfire badly on converted content. See `docs/wiki/mdbook-lint-bugs.md` for the catalogue. Running both, with each scoped to what it does well, gives a clean run without disabling checks that matter.

## What Each Tool Owns

`mdbook-lint` is kept for exactly six checks:

| Rule | Checks |
|---|---|
| MDBOOK003 | `SUMMARY.md` structure |
| MDBOOK004 | Duplicate chapter titles across the book |
| MDBOOK005 | Files under `docs/` not referenced in `SUMMARY.md` |
| MDBOOK023 | Chapter title disagreeing with the page's H1 |
| CONTENT007 | Terminology consistency, inferring the majority variant per document |
| ADR001-ADR017 | Architecture-decision-record structure |

MDBOOK005 is the one that most repays the cost of running a second linter: an unreferenced page renders nowhere and is easy to miss while porting. The ADR rules are unused today but `docs/decisions/` is provisioned in the LID setup, so they are ready when it fills up.

`rumdl` owns everything else — all 53 markdownlint rules plus 28 of its own, and it fixes most of what it finds. `mdbook-lint` has a `--fix` flag too, but only two of its rules implement a fix and none of the `MDBOOK*` ones do, so in practice it reports rather than repairs.

Deliberately left to `rumdl` rather than `mdbook-lint`, because `mdbook-lint`'s versions produce false positives on ported pages:

- **MD049** over MDBOOK-side emphasis checking. Converted tables contain escaped underscores (`\_cindex`) that `mdbook-lint` misreads as emphasis. This escaping is unavoidable upstream, so the rule has to come from the tool that handles it correctly.
- **MD063** over CONTENT004 for heading capitalization. CONTENT004 locks a whole document's expected style from its first ambiguous heading and flags every other heading against it. MD063 is off by default and stays off.
- **MD040** over MDBOOK001, **MD057** over MDBOOK002 and MDBOOK006, **MD059** over CONTENT010, **MD061** over CONTENT001 — all straight duplicates where `rumdl`'s version is at least as good and fixable.

Not enabled from `mdbook-lint` because these docs contain no mdBook directives and no Rust: MDBOOK007, MDBOOK008, MDBOOK009, MDBOOK010, MDBOOK011, MDBOOK012, MDBOOK016, MDBOOK017, MDBOOK021, MDBOOK022.

## Rules Disabled in `docs/.mdbook-lint.toml`

Each entry is a rule that fires on correct content in this project:

- **MD013** (line length) — these docs run long, dense lines by convention.
- **CONTENT001** ("BUG" text found) — several pages exist specifically to document real bugs.
- **CONTENT002** (placeholder text) — a keyword scanner that fires on the word "placeholder" in ordinary prose.
- **CONTENT003** (short chapter) — fires on `SUMMARY.md`, which is structure, not prose. Padding it risks breaking mdBook's strict parser.
- **CONTENT011** (future tense) — correct usage in `roadmap.md` and other forward-looking sections.
- **MDBOOK010** (inline math) — counts raw dollar signs document-wide without excluding code spans. These docs contain no math.

## Running It

`mise run lint-markdown` runs both linters. They are independent tasks (`rumdl-lint` and `mdbook-lint`) so both always report, and the aggregate fails if either does. The same pair runs as `prek` hooks, scoped by path so they only fire when `docs/` markdown or a linter config changes.

Each linter is configured where it looks by default: `.rumdl.toml` at the repo root, and `docs/.mdbook-lint.toml` passed explicitly via `-c` because the standalone `mdbook-lint` CLI does not auto-discover it.

Both are scoped to the same paths — `docs/SUMMARY.md`, `docs/wiki/`, `docs/reference-repos.md` — and both leave the LID design tree alone, since it follows LID's conventions rather than this project's. They arrive there differently: `rumdl` has an `include` list in its config, while `mdbook-lint` has no working exclusion mechanism and must be handed the paths as arguments.

Two project-wide rule exemptions live in `.rumdl.toml` rather than being fought page by page: MD013, because these docs run long dense lines by convention, and MD064, because two spaces after a sentence is house style here. `SUMMARY.md` additionally exempts MD025, since mdBook uses repeated level-1 headings there as part dividers.

`cache = false` is set in `.rumdl.toml` for the reason in "Fixing, Not Just Reporting" below — it is the config equivalent of always passing `--no-cache`.

## Invocation Notes

`mdbook-lint` must be given an **absolute** path. MDBOOK005 mis-scopes its orphan scan to the process's working directory instead of the book's source directory when handed a relative one, reporting every stray Markdown file on disk as orphaned. The `mdbook-lint` mise task resolves an absolute path before passing it.

There is no way to exclude a subtree by config — `ignore_paths` appears in upstream's own example config but is never read anywhere in the source. Exclusion works only by passing the paths you *do* want linted as explicit arguments, which is how the LID design tree (`docs/intent/`, `docs/arrows/`, `docs/high-level-design.md`) stays out of scope.

Do not wire `mdbook-lint` in as a `[preprocessor.lint]` in `book.toml`. That path accepts neither the config file nor path scoping, so it lints the whole tree and fails `mdbook build` outright on any error-severity finding in the excluded design tree.

## Fixing, Not Just Reporting

`rumdl fmt` applies mechanical fixes; `rumdl check --fix` does the same but exits non-zero if anything unfixable remains, which is the right choice for CI. Preview with `--diff` before applying to freshly converted pages.

Caching is disabled in `.rumdl.toml`. `rumdl` invalidates a cached result only when the linted Markdown file changes, but MD057 and MD051 also read link targets on disk, so adding or removing a linked asset would otherwise leave a stale verdict in place — see `docs/wiki/rumdl-bugs.md`. At this tree's size the cache saves nothing worth the confusion; pass `--no-cache` too if you invoke `rumdl` from somewhere the config isn't discovered.

One fix is worth overriding by hand. Where a list item's continuation line is under-indented, `rumdl fmt` resolves it by inserting a blank line, which turns a tight list item into a loose one with two paragraphs and changes the rendered output. Re-indent the continuation to three spaces instead. Converting with the settings in `docs/wiki/converting-html-to-markdown.md` avoids producing this shape at all.

## Evaluated Alternatives

[`mado`](https://github.com/akiomik/mado) was evaluated against `rumdl` in August 2026 and rejected. It implements 38 rules to `rumdl`'s 119, has no auto-fix of any kind, and releases roughly annually. Decisively, it reports a false-positive MD029 against every item of an ordered list whose earlier items wrap onto a continuation line — 35 occurrences in a single ported page — where both `rumdl` and the reference `markdownlint` implementation are silent:

```markdown
# T

1. one
   wrapped
2. two
```

`markdownlint-cli2` remains the correctness reference. When `rumdl` and `mdbook-lint` disagree about a general-purpose rule, it is the tiebreaker; it is a Node tool and is not part of the normal workflow.
