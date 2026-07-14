# compiledb Bugs Encountered

Bugs found while using [`compiledb`](https://github.com/nickdiego/compiledb) (via `uv run --with compiledb`, no pinned reference clone yet — see the followup task below) in `packaging/lint_cpp.py`'s compilation-database generation for `clang-tidy`/`clazy`. Unlike `docs/wiki/mdbook-lint-bugs.md`'s entries, this one hasn't had its root cause source-read yet, and hasn't been checked against upstream issues yet either — that's the followup task itself.

## 1. `-n`/`--no-build` Mode Drops the Shell-Escaped Quotes Around a String-Literal `-D` Define

**Symptom:** `compiledb -n -o compile_commands.json make` (parsing a `make -n` dry run rather than a real build, which is what makes it fast — see `packaging/lint_cpp.py`'s `generate_compile_db()`) generates a `compile_commands.json` entry for `-DQT_TESTCASE_BUILDDIR=...` with its value unquoted: `-DQT_TESTCASE_BUILDDIR=/home/jmbowman/Source/portabase/tests/dbeditor` instead of the shell-escaped `-DQT_TESTCASE_BUILDDIR='"/home/jmbowman/Source/portabase/tests/dbeditor"'` that qmake's generated `Makefile` actually prints and that a real build actually receives. `clang-tidy`, fed the malformed argument, parses the path's `/` and `-` separated components as bare identifiers rather than a string literal, producing spurious `use of undeclared identifier` errors everywhere the macro is used — in this project, inside Qt's `QTEST_MAIN` macro (which passes the source file's path and `QT_TESTCASE_BUILDDIR` to `QTest::setMainSourcePath`), on every `CONFIG += testcase` qmake project (i.e. any QtTest suite, not specific to this one).

**Root cause:** not yet source-read. Inferred from behavior: `compiledb -n` works by parsing `make -n`'s printed command-line text rather than capturing real process `argv` (that's the source of its speed advantage over tools like `bear`, which do intercept real `exec()` calls), so it has to re-tokenize the printed shell command line itself — and that re-tokenization appears to mishandle this specific quote-within-quote pattern (`'"..."'`, a literal double-quote preserved through a shell single-quote), stripping both layers instead of preserving the inner one.

**Workaround:** this project's `packaging/lint_cpp.py` post-processes the generated `compile_commands.json` afterward (`_fix_unquoted_string_defines()`), detecting any `-DQT_TESTCASE_BUILDDIR=` argument missing its trailing quote and re-quoting it in place before `clang-tidy`/`clazy` run.

**Minimal reproduction:** any qmake project with `CONFIG += testcase` (this project's `tests/dbeditor/dbeditor_test.pro` is one) run through `compiledb -n -o compile_commands.json make` — qmake auto-generates the `-DQT_TESTCASE_BUILDDIR='"..."'` define for any such project, so this isn't specific to PortaBase.

**Checked for an existing issue:** not yet checked. **Status: not yet filed.**

## Followup Task: Report Upstream

1. **Confirm it hasn't already been reported** — search [compiledb's GitHub issues](https://github.com/nickdiego/compiledb/issues) (open and closed) for this symptom before filing anything new.
2. If not already reported: pin a `compiledb` clone via `docs/reference-repos.yaml` (see `docs/reference-repos.md` for the mechanism) and source-read the actual tokenizer/parser responsible, to replace the "inferred from behavior" root cause above with a real one — matching the standard set in `docs/wiki/mdbook-lint-bugs.md`'s entries.
3. File the issue upstream with the minimal reproduction above, cross-linking any related existing issue found in step 1.
