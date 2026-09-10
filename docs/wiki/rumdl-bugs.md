# rumdl Bugs Encountered

Bugs found while using [`rvben/rumdl`](https://github.com/rvben/rumdl) in this project, investigated against the reference clone (`docs/reference-repos.md`, `.reference-repos/rumdl/`, pinned to `v0.2.48`, the version this project runs). Draft material for filing upstream issues. See `docs/wiki/markdown-linting.md` for how `rumdl` is used here.

## 1. The Result Cache Ignores Files a Rule Reads Other Than the Linted File

**Symptom:** MD057 kept reporting `Relative link 'images/portabase_old.png' does not exist` for a file that did exist. The report survived re-running the linter, touching the Markdown file, and passing an absolute path. Only `--no-cache` produced the correct result.

**Root cause (behavioral, not source-read):** `rumdl` caches per-file lint results and invalidates an entry when that Markdown file changes. MD057 and MD051, though, reach outside the file being linted — they stat link targets on disk. Creating (or deleting) a link target changes what those rules should report without changing the Markdown file at all, so the stale cached verdict is served indefinitely.

The failure mode is asymmetric and unhelpful in both directions: a link fixed by adding the missing file keeps being reported as broken, and a link broken by deleting a file keeps being reported as fine.

**Workaround:** pass `--no-cache` whenever link-validation results matter — particularly right after adding, moving, or deleting linked assets. Alternatively delete the `.rumdl_cache/` directory. It is gitignored here.

Note the linted Markdown file's own mtime is *not* a sufficient trigger — `touch` on it did not clear the stale entry in testing, so the cache key appears to be content-based.

**Checked for an existing issue:** not exhaustively searched. **Status: not yet filed.**

## Non-Bugs Worth Recording

**MD057 resolves relative links against the linted file's directory, not the working directory.** This was initially misdiagnosed as a path-resolution bug, because running from the file's own directory appeared to fix it while running from the repository root did not. A minimal reproduction behaved correctly from both, and `--no-cache` explained the difference: the two runs were reading different cache states, not resolving paths differently. Resolution is file-relative as documented.
