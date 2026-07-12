# mdbook-lint Bugs Encountered

Bugs found while using [`joshrotenberg/mdbook-lint`](https://github.com/joshrotenberg/mdbook-lint) in this project, investigated against the reference clone (`docs/reference-repos.md`, `.reference-repos/mdbook-lint/`, pinned to `v0.14.4`, the version this project runs). Checked against [GitHub issues](https://github.com/joshrotenberg/mdbook-lint/issues) before filing anything new — status noted per bug. Draft material for filing upstream issues.

## 1. MDBOOK005's Orphan-Scan Ignores the Book Src Dir When Given a Relative Path

**Symptom:** `mdbook-lint lint docs` (a relative path) reports orphaned-file warnings for files far outside `docs/` — `.reference-repos/`, `apm_modules/`, anything else under the process's cwd — none of which are part of this book.

**Root cause (source-read), `crates/mdbook-lint-rulesets/src/mdbook/mdbook005.rs`, `check_with_ast`:**

```rust
let book_src_dir = if document.path.is_absolute() {
    document.path.parent().unwrap_or(Path::new("."))
} else {
    Path::new(".")
};
```

When the `SUMMARY.md` document's path is relative (as it is whenever the CLI is invoked with a relative path argument), the scan falls back to the *process's cwd*, not `SUMMARY.md`'s own directory. This directly contradicts the rule's own doc comment ("Only scans within the directory containing SUMMARY.md, not parent or sibling directories") and is either a regression of, or an incomplete fix for, upstream issue #142 ("MDBOOK005 orphaned files detection extends beyond book src directory") — reported and reportedly fixed by #145 back in August 2025, but the relative-path branch still exhibits the exact original symptom.

**Workaround:** always invoke `mdbook-lint lint` with an absolute path (see this project's `mdbook-lint` mise task in `mise.toml`, which resolves one via a shell variable before passing it as the argument).

**Checked for an existing issue:** #142/#145 cover the exact symptom but are closed as fixed; this looks like a regression, or a branch #145 didn't cover. #412 ("Ignore patterns does not work for e.g. MDBOOK005") is a separate, currently-open bug in the same rule — possibly related maintenance gap, not the same root cause. **Status: not yet filed** — worth cross-linking #142 when filed.

## 2. `ignore_paths` Global Config Option Is Documented but Never Implemented

**Symptom:** mdbook-lint's own dogfooded example config (`docs/.mdbook-lint.toml` in its repo) shows a commented-out `# ignore_paths = ["target/", "drafts/"]` line under global settings. Setting this key in a real config has no effect on anything.

**Root cause (source-read):** `grep -rn "ignore_paths" crates/` across the entire mdbook-lint source tree returns zero matches. The key exists only as a commented-out example in documentation — it's never parsed or applied anywhere in the codebase.

**Workaround:** there's no way to exclude a subtree from linting via config. Pass only the paths you want linted as explicit CLI arguments instead — directories not passed are simply never visited. This project's `mdbook-lint` mise task does exactly this to exclude the LID design tree (`docs/intent/`, `docs/arrows/`, `docs/high-level-design.md`), which follows LID's own formatting conventions rather than this project's.

**Checked for an existing issue:** not exhaustively searched. **Status: not yet filed.**

## 3. CONTENT004 Defaults to Sentence Case for Short/Ambiguous Headings, Overriding an Otherwise Title-Case Document

**Symptom:** a heading like `# Agentic SDLC` (two words, the second a bare acronym) sets the *whole document's* expected heading-capitalization style to sentence case — even though every other heading in the file is genuinely Title Case — and then flags all of them as violations.

**Root cause (source-read), `crates/mdbook-lint-rulesets/src/content/content004.rs`, `detect_style`:**

```rust
fn detect_style(&self, text: &str) -> CapitalizationStyle {
    let is_title = self.is_title_case(text);
    let is_sentence = self.is_sentence_case(text);
    if is_title && !is_sentence {
        CapitalizationStyle::TitleCase
    } else {
        CapitalizationStyle::SentenceCase
    }
}
```

When a heading has too few "significant" words to distinguish the two styles — e.g. because its only non-first word is an all-caps acronym, or a word on the small `TITLE_CASE_EXCEPTIONS` list (articles/prepositions) — both `is_title_case` and `is_sentence_case` independently return `true`, and the tie-break silently prefers `SentenceCase` regardless of the rest of the document. `check_with_ast` sets `detected_style` once, from the *first* multi-word heading encountered, and never reconsiders it — so this one ambiguous heading (if it happens to be first) locks in the wrong baseline for the entire file.

A related quirk in the same word-classification logic (`get_significant_words`): any token whose first character isn't alphabetic — e.g. `(Agent`, a parenthesis-prefixed word — is excluded from significant-word counting entirely, so parenthetical asides silently dodge the capitalization check regardless of their own case. This is why `Conclusion (as of 2026-07-06)` and `Obsidian Bases (new core plugin, early 2026)`-style headings behave unintuitively: the parenthetical's leading word doesn't count, but interior words might.

**Workaround:** avoid short/acronym-only/preposition-only headings as the *first* multi-word heading in a document, if the rest of the document is Title Case — pad them with a real content word (e.g. `# Agentic SDLC` → `# Agentic SDLC Wishlist`) so `is_sentence_case` correctly evaluates to `false` and title case wins the tie-break.

**Checked for an existing issue:** not exhaustively searched. **Status: not yet filed.**

## 4. Wiring in as `[preprocessor.lint]` Re-Lints the Whole Tree Unscoped, and Hard-Fails the Build on Any Error-Severity Finding

**Symptom:** adding `[preprocessor.lint]` to `book.toml` makes `mdbook build` invoke mdbook-lint against every file under `src`, with no way to pass the same path-exclusion or `-c <config>` scoping available to the standalone CLI. If any excluded content — in this project's case, the LID design tree — has `error`-severity findings (e.g. `MD052`, `MD003`), `mdbook build` exits non-zero and the whole book fails to build.

**Root cause:** not source-read in depth — inferred from behavior. The preprocessor invocation path clearly doesn't accept the same CLI args or config-file discovery as `mdbook-lint lint`, and mdBook's preprocessor protocol itself has no path-scoping mechanism to offer it.

**Workaround:** don't use the preprocessor integration if any part of the book needs to be excluded from linting. Run `mdbook-lint` as a separate, explicitly-scoped step instead (this project's `mise run mdbook-lint` task) and keep `book.toml` free of `[preprocessor.lint]`.

**Checked for an existing issue:** not exhaustively searched. **Status: not yet filed.**

## 5. MDBOOK010 (Inline Math Block) Counts Raw `$` Characters Document-Wide, Including Inside Code Spans

**Symptom:** hit while drafting this very page. A single literal `$` inside a backtick-wrapped code span (e.g. `` `$PWD/docs` ``) — with no intent to write math at all — produced an "unclosed inline math block" error for the whole document, and cascaded to every paragraph after it once the running total went odd.

**Root cause:** not source-read. Inferred from behavior: the rule appears to count `$` occurrences across the raw document text to detect balanced math-block delimiters, without excluding text inside inline code spans or fenced code blocks first. Any odd total — including from shell-variable syntax (`$PWD`, `$HOME`, etc.) inside backticks — triggers a false "unclosed" error.

**Workaround:** disabled outright in this project's `docs/.mdbook-lint.toml` (`disabled-rules = ["MDBOOK010", ...]`) — not worth fighting for a project that doesn't write math. If math support is ever needed, avoid writing shell variables with a literal `$` inside backticks in prose instead of disabling the rule; rephrase around it (e.g. "a shell variable" instead of `` `$PWD` ``).

**Checked for an existing issue:** not exhaustively searched. **Status: not yet filed.**
