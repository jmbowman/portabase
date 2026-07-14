#!/usr/bin/env -S uv run --script
# /// script
# requires-python = ">=3.11"
# ///
"""Lint C/C++ files changed vs. a base ref, not the whole codebase.

To be run from the project root, like "uv run packaging/lint_cpp.py"
Assumes cppcheck is on the PATH.

Lints only C/C++ files that differ from a base ref (default: origin/master,
override with --base-ref or the BASE_REF env var) plus any locally
modified/untracked files, running cppcheck against each.

clang-tidy and clazy are deliberately not wired in here for now -- they need
a compile database (clang-tidy/clazy analyze .cpp translation units, not
standalone files the way cppcheck does), and this project's test suite and
app currently recompile the same shared sources into two separate qmake
targets with different flags, which is exactly the kind of setup where
generating one is more trouble than it's worth. Revisit alongside the
planned app/test library split (see docs/wiki/dbeditor-code-review-
followups.md's "Build/Test Infrastructure Followups") and a comparative
look at other C++ linters; docs/wiki/compiledb-bugs.md documents a real
compiledb defect worth re-checking whenever clang-tidy/clazy return.
"""

import argparse
import os
import subprocess
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
EXCLUDE_PREFIXES = ("metakit/", "src/encryption/randomkit/")
CPPCHECK_SUPPRESSIONS = "packaging/cppcheck/suppressions.txt"
TESTS_ROOT = REPO_ROOT / "tests"


def test_pro_files():
    """Discover test projects: one tests/*/*.pro file per test project.

    Matches the one-project-per-subdirectory convention portabase-tests.pro's
    own SUBDIRS list follows (one entry per test project), without needing to
    parse qmake's SUBDIRS list syntax.
    """
    return sorted(TESTS_ROOT.glob("*/*.pro"))


def run(cmd, cwd=REPO_ROOT, quiet=False, check=True):
    # stdin=DEVNULL is defense in depth: nothing this script runs should
    # ever need interactive input, and a child process blocking on stdin
    # (e.g. git invoking a pager if its non-interactive detection doesn't
    # fire under whatever's orchestrating this script) should fail fast
    # rather than hang forever. quiet suppresses both stdout and stderr --
    # cppcheck (and some other tools) write their actual findings to
    # stderr, not stdout, so only silencing stdout would leave "quiet"
    # passes printing anyway.
    devnull = subprocess.DEVNULL if quiet else None
    return subprocess.run(
        cmd,
        cwd=cwd,
        check=check,
        stdin=subprocess.DEVNULL,
        stdout=devnull,
        stderr=devnull,
    )


def changed_files(base_ref):
    patterns = ["--", "*.cpp", "*.h", "*.c"]
    commands = [
        ["git", "--no-pager", "diff", "--name-only", "--diff-filter=ACMR", f"{base_ref}...HEAD", *patterns],
        ["git", "--no-pager", "diff", "--name-only", "--diff-filter=ACMR", "HEAD", *patterns],
        ["git", "--no-pager", "ls-files", "--others", "--exclude-standard", *patterns],
    ]
    files = set()
    for cmd in commands:
        result = subprocess.run(
            cmd, cwd=REPO_ROOT, capture_output=True, text=True, check=True, stdin=subprocess.DEVNULL
        )
        for line in result.stdout.splitlines():
            line = line.strip()
            if line and not line.startswith(EXCLUDE_PREFIXES):
                files.add(line)
    return sorted(files)


def run_cppcheck(files):
    test_include_flags = [f"-I{p.parent.relative_to(REPO_ROOT)}" for p in test_pro_files()]
    include_flags = ["-Isrc", *test_include_flags, "-Imetakit/include"]

    # --language=c++ is required here specifically because `files` can
    # include bare .h paths (changed_files() matches *.h too): cppcheck
    # infers language per file it's directly given, and for a standalone
    # .h argument it defaults to C rather than C++ -- unlike scanning a
    # whole directory (as packaging/cppcheck/cppcheck.sh does), where
    # headers are only ever reached via inclusion from an already-C++ .cpp
    # file and so are never misdetected. Without this flag, a changed
    # header that #includes <mk4.h> (e.g. via database.h/dbeditor.h)
    # fails with a bogus syntaxError on Metakit's `class c4_View {` --
    # valid C++, invalid as C. Every file this project passes to cppcheck
    # is genuinely C++: the only .c sources are vendored randomkit files,
    # already excluded via EXCLUDE_PREFIXES.

    # Broad, informational pass: reports everything (style/performance/
    # portability/information), including known pre-existing debt in
    # legacy files touched incidentally by a change. Never blocks --
    # --error-exitcode isn't set here.
    run([
        "cppcheck", "--language=c++", "--enable=all", "--inconclusive", "--library=qt",
        f"--suppressions-list={CPPCHECK_SUPPRESSIONS}", "-q",
        *include_flags, *files,
    ], check=False)

    # Strict pass: cppcheck's default check set (error/warning severity
    # only, no --enable=), which currently has zero findings across this
    # project's changed-file history (see
    # docs/wiki/dbeditor-code-review-followups.md). Output is a subset of
    # the broad pass above, so run it quietly and use only its exit code.
    strict = run([
        "cppcheck", "--language=c++", "--library=qt", "--error-exitcode=1",
        f"--suppressions-list={CPPCHECK_SUPPRESSIONS}", "-q",
        *include_flags, *files,
    ], quiet=True, check=False)
    return strict.returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--base-ref", default=os.environ.get("BASE_REF", "origin/master"))
    args = parser.parse_args()

    files = changed_files(args.base_ref)
    if not files:
        print("No changed C/C++ files to lint.")
        return 0

    print("Linting:")
    for f in files:
        print(f"  {f}")
    # Flush explicitly: stdout is fully buffered (not line-buffered) once
    # it's a pipe/file rather than a TTY -- exactly how prek/CI capture hook
    # output -- so without this, cppcheck's subprocess (writing straight to
    # the inherited stdout fd, bypassing Python's buffer) would print its
    # findings before this banner ever reaches the same stream.
    sys.stdout.flush()

    return run_cppcheck(files)


if __name__ == "__main__":
    sys.exit(main())
