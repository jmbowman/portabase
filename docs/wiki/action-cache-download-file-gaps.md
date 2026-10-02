# action-cache-download-file Feature Gaps

The Android CI job installs the Qt-5.15-compatible NDK (r21e, 1.19GB) by downloading the archive directly, verifying its checksum, and caching the result — see the `android-build` job in `.github/workflows/build.yml`. [`mercury233/action-cache-download-file`](https://github.com/mercury233/action-cache-download-file) (v1.3.0) does almost exactly that as a declarative step, and adopting it would replace the hand-rolled shell. This page records what still stands in the way, as draft material for an upstream pull request: the action is a composite action, so each change below is a few lines of shell.

Unlike the other pages in this section, these are missing features rather than defects — the action does what it documents.

**What it already does well**, and why it is worth contributing to rather than replacing: it is plain auditable shell with no bundled JavaScript; it validates its URL, filename, and destination inputs carefully; it verifies SHA-256 against both freshly downloaded and cached copies; and it uses the expected checksum as the cache key, which is a neat content-addressed touch.

## Why the NDK Archive Is Cached

Caching this download is a reliability measure, not just a speed optimization. Every cache hit is a 1.19GB transfer from a third-party mirror that does not happen — and a corrupt or truncated transfer of that archive is a recurring cause of Android build failures (issue #80). Skipping the fetch removes the failure mode rather than retrying it.

Two facts about GitHub's cache make this worthwhile here, and both are easy to get wrong:

- **Cache entries are scoped by ref.** A run restores caches created on its own ref or on the default branch; a `pull_request` run's cache is scoped to `refs/pull/N/merge`, which `master` cannot read. So a merge to `master` does *not* inherit the cache its pull request just populated. What does hit: every subsequent push to an open pull request (same scope), and any pull request opened within the eviction window of the last `master` run (default-branch caches are readable from every branch).
- **Entries untouched for 7 days are evicted**, against a 10GB per-repository limit. PortaBase's cache usage is around 0.4GB across five entries, so the NDK archive plus a second ref-scoped copy still leaves the repository well under a third of its quota. There is nothing valuable for it to displace.

## 1. Retries Restart the Download Instead of Resuming

**Gap:** the download step runs `curl --retry 5 --connect-timeout 30 --location --globoff` with no `--continue-at -`, so each retry re-fetches the file from byte zero.

**Why it matters here:** a transfer of a 1.19GB archive that dies at 1.1GB restarts from nothing, and five such attempts can exhaust the job's time budget without ever completing — at that size the retry count works against you. Resuming turns the same five attempts into five chances to finish.

**Proposed change:** add `--continue-at -` to the curl invocation. It is safe to pair with the action's existing checksum step, which catches a corrupt resume; and on a fresh runner with no partial file, curl treats it as an ordinary download.

## 2. Only SHA-256 Is Accepted for Verification

**Gap:** the `sha256` input is the only integrity option.

**Why it matters here:** the checksum Google publishes for the NDK archive, in sdkmanager's own `repository2-3.xml` manifest, is a SHA-1. Supplying a SHA-256 means computing it from a copy we downloaded ourselves, which verifies that later downloads match our first one rather than matching what the vendor published — weaker provenance for the same amount of work.

**Proposed change:** accept an optional `sha1` input alongside `sha256`. Lower priority than the gap above, and the more likely of the two to be reasonably declined: SHA-1 is adequate for detecting corruption but not for tamper resistance, and a maintainer may not want a weak hash in a verification path. A self-computed SHA-256 is an acceptable fallback if so.

## Adoption Condition

Gap 1 is the blocker. If it lands upstream, this action becomes a better choice than the inline `curl` plus `sha1sum` plus `actions/cache` steps, and the NDK install in `.github/workflows/build.yml` should switch to it. Gap 2 is not a blocker — a self-computed SHA-256 works.
