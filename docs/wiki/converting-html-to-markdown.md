# Converting HTML to Markdown

How an HTML page is converted into a Markdown page under `docs/wiki/`, and the manual follow-up it needs afterwards. Written for porting the old PortaBase website, but the recipe is general — the only page-specific parts are the CSS selector and the heading shift.

Linting of the result is covered separately in `docs/wiki/markdown-linting.md`.

## Tools

- [`html-to-markdown`](https://github.com/xberg-io/html-to-markdown) (`v3.10.2`, installed via `cargo`) does the conversion. Reference clone at `.reference-repos/html-to-markdown/`.
- `packaging/html_extract.py` narrows the HTML to the real content region and normalizes headings first. A `uv` script with inline PEP 723 dependencies, like `packaging/lint_cpp.py`.
- `rumdl` cleans up mechanical residue afterwards.

## The Recipe

```bash
uv run packaging/html_extract.py PAGE.html \
    -s '#content' --shift-headings -1 --promote-bold-paragraphs 4 \
  > /tmp/PAGE-content.html

html-to-markdown -o docs/wiki/PAGE.md \
    --strip-newlines -w --wrap-width 80 \
    --code-block-style backticks --show-warnings \
  /tmp/PAGE-content.html
```

Then add the page to `docs/SUMMARY.md` and work through the manual checks below.

## Why Each Setting

**`--strip-newlines` is the important one.** The legacy pages are hand-wrapped inside their `<li>` elements. Without this flag, `html-to-markdown` preserves those newlines and indents the continuation lines by a single space, producing list items that most linters misparse. On the file-format page it alone accounted for 35 findings. The flag replaces newlines with spaces before conversion, and `-w --wrap-width 80` then re-wraps to correctly indented continuations.

**`--code-block-style backticks`** overrides the default of indented code blocks, which cannot carry a language tag and so can never satisfy MD040.

**`--show-warnings`** surfaces non-fatal conversion problems that are otherwise silent.

Extracting the content region with a CSS selector beats `html-to-markdown`'s own `-p --preset aggressive` preprocessing. The preset is heuristic: on the file-format page it correctly dropped the nav and the SourceForge footer, but left the site banner and copyright line behind. A selector is deterministic, and the legacy site's consistent template makes `#content` reliable across pages. Confirm the selector per page before trusting it.

`--shift-headings -1` is needed because the site's `<h1>` is the banner, so each page's real title is an `<h2>`. Without the shift every heading sits one level too deep and the page has no H1.

`--promote-bold-paragraphs 4` handles the `<p><b>Version 11</b></p>` pattern the legacy pages use for subheadings. These convert to bare emphasis, which is both wrong semantically and an MD036 finding — eleven of them on the file-format page. Pick the level that nests correctly under the surrounding real headings rather than assuming a fixed one.

### Two Known Bugs in `packaging/html_extract.py`

Both found 2026-08-18 and confirmed by reproduction; neither is fixed yet, and neither damaged the already-ported pages. Scheduled first among the outstanding fixes — see the Remediation Plan in `docs/wiki/integritycheck-code-review-followups.md` — because they are the only ones that keep causing new damage with every further page ported.

**`--promote-bold-paragraphs` emits a heading one level shallower than asked.** `main()` promotes before shifting, so the shift applies to the promoted heading too: with `--shift-headings -1`, asking for level 4 emits `<h3>`. The file-format page came out correct *by accident* because that off-by-one happened to land where the nesting needed it. Until this is fixed, check the emitted level rather than trusting the argument. Fix: shift first, then promote — or exempt promoted headings from the shift.

**`BOLD_PARAGRAPH_RE` can span paragraphs and mangle them.** The pattern uses a lazy `.*?` under `DOTALL` between `<p>\s*<b>` and `</b>\s*</p>`, so it does not require the paragraph's entire content to be bold. Given `<p><b>Bold intro</b> text</p>` followed by `<p>more <b>bold end</b></p>`, it matches across both and produces `<h4>Bold intro</b> text</p>\n<p>more <b>bold end</h4>` — two paragraphs merged into one heading with broken inner tags. The same failure occurs inside a single paragraph holding two bold runs where the last one ends it. The legacy pages ported so far happen to have only wholly-bold paragraphs, which is why it never fired. Fix: forbid `</p>` (and an intervening `<b>`) inside the captured text.

### Settings Deliberately Not Used

**`--extract-metadata`** emits YAML frontmatter. mdBook has no frontmatter support whatsoever, so it would render as literal text between horizontal rules.

**`--list-indent-width`** does not do what it sounds like here. It governs nested-list indentation only; values of 2, 3 and 4 produce byte-identical continuation lines. It is not a lever on the wrapped-list problem.

## What Cannot Be Fixed by Configuration

Table cells always escape `*` and `_`, so `_cindex` becomes `\_cindex`. No flag disables this — the escape flags are bypassed for the table-cell path. It is harmless when rendered, but `mdbook-lint`'s MD049 misreads it as emphasis, which is one reason `rumdl` owns that rule.

## Manual Checks After Converting

Run `rumdl check` on the new page. What remains should be content problems, not formatting ones. On the file-format page the recipe left 13 findings, all of them genuine:

- **Dropped anchors (MD051).** The legacy pages use `<a name="versionmap">` targets, which do not survive conversion, so in-page links like `[here](#versionmap)` break. Retarget each to the generated heading anchor.
- **Non-descriptive link text (MD059).** The same links usually read "here". Rewrite them while retargeting.
- **Missing alt text (MD045).** Absent in the source HTML; write real descriptions rather than empty brackets.
- **Links to unported pages (MD057).** Expected while porting is in progress; these resolve as more pages land. Verify each one is genuinely pending rather than a typo.

### Assets the Page Links To

Images, schemas, stylesheets and anything else the page references have to live under `docs/` to survive the build — mdBook copies static files from the book source into the output, and ignores anything outside it. A relative path climbing out of `docs/` renders as a dead link. Symlinks do work, but they break on Windows clones and pull the whole target directory into every build, so copies are the safer default. Images go in `docs/wiki/images/`, other linked assets in `docs/wiki/assets/`.

### Images in Table Cells

mdBook wraps every image in a block-level `<label>` for its click-to-zoom feature, so two images in one table cell render on separate lines with the separator stranded between them. `docs/custom.css` overrides this to `inline-block` inside tables, wired up via `additional-css` in `docs/book.toml`.

Then check by eye, since no linter covers these:

- Tables converted with their columns intact, and no cell content silently flattened.
- Code spans and blocks did not lose significant whitespace.
- The selector did not clip real content at either end of the page.
- Heading nesting reads sensibly after the shift and any bold-paragraph promotion.

## Re-converting an Already-Ported Page

Worth doing for any page ported before this recipe existed. `docs/wiki/format.md` was originally converted with `-p` alone and hand-edited afterwards; it carried 45 `rumdl` findings. Re-converting it through the recipe and re-applying those hand edits brought it to one — the single genuine link to a page not yet ported.

Convert to a scratch file and diff against the committed version rather than overwriting it. Hand edits are easy to lose, and they are the part worth keeping: compare the two word-by-word rather than line-by-line, since re-wrapping makes a line diff unreadable. Everything the earlier pass fixed by hand — descriptive link text, retargeted anchors, image alt text, heading case — has to be carried across, because conversion reintroduces all of it.
