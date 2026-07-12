---
name: reference-repos
description: Local read-only clones of external repositories kept for fast, exact reference (e.g. checking how an upstream dependency or comparable project actually behaves, instead of relying on recall or a web fetch). Use this skill to find out which reference repos a project has available, where they live on disk, when to consult them, and how to add, remove, or refresh them.
---

# Reference repos

A convention for keeping local, read-only, shallow clones of external repositories around for fast, exact answers — checked out once, refreshed cheaply, never edited.

## Precedence: clone before docs site

Check the local clone first for anything about exact behavior, config, schema, or CLI flags — it's pinned to the exact version in use, needs no network round-trip, and for most tools the public docs site is itself generated from a `docs/` (or similar) folder inside the same clone, so the clone usually *is* the docs, plus the source to resolve it against when they disagree. Reach for the public docs site only for onboarding/conceptual material (why the tool exists, recommended workflows) that reads better as curated prose than reverse-engineered from source — and even that's often sitting in the clone's own README/docs tree already.

## Layout

The manifest and project-notes file live in `docs/`:

- **`docs/reference-repos.yaml`** — machine-managed by [vcs2l](https://github.com/ros-infrastructure/vcs2l) (installed via `mise`). Pure `type`/`url`/`version` data. **Never hand-edit** — it's regenerated wholesale by `vcs export`, which silently drops any comments or extra fields added to it.
- **`docs/reference-repos.md`** — project-specific and human/agent-maintained: which repos are checked out here and when to consult each one. Read this file to find out what's currently available and why.
- **`.reference-repos/<name>/`** — the actual clones, at the repo root regardless of where the manifest/notes live. Gitignored. **Read-only** — never edit or commit to anything here; refreshes overwrite local state, and edits would silently vanish. If a change needs testing against one of these repos, copy it elsewhere first.

If a project has this skill installed but no manifest/notes file yet, bootstrap them (empty manifest `repositories: {}`, and a notes file with a "Current repos" section) plus the three mise tasks below, adjusting paths to match wherever the project keeps this kind of file.

## mise tasks

```toml
[tasks.reference-repos-setup]
description = "Shallow-clone any repos in docs/reference-repos.yaml not already present in .reference-repos/"
run = "mkdir -p .reference-repos && vcs import --shallow --skip-existing --input docs/reference-repos.yaml .reference-repos"

[tasks.reference-repos-update]
description = "Fast-forward pull all repos in .reference-repos/"
run = "vcs pull .reference-repos"

[tasks.reference-repos-regenerate]
description = "Regenerate docs/reference-repos.yaml from the actual contents of .reference-repos/"
run = "vcs export .reference-repos > docs/reference-repos.yaml"
```

(Also requires `"pipx:vcs2l" = "latest"` under `[tools]`.)

## Adding a repo

1. Shallow-clone it directly into place:
   ```
   git clone --depth 1 [--branch <branch>] <url> .reference-repos/<name>
   ```
2. Regenerate the manifest from disk state:
   ```
   mise run reference-repos-regenerate
   ```
3. Add a section for it in `docs/reference-repos.md`: what it is, and when an agent should consult it.

## Removing a repo

```
rm -rf .reference-repos/<name>
mise run reference-repos-regenerate
```

Then delete its section from `docs/reference-repos.md`.

## Refreshing

Clones go stale. Refresh all of them with:

```
mise run reference-repos-update
```

Fast-forward only, bounded fetch (shallow clones stay shallow — no history growth, no merge logic, since these clones are never modified locally). Safe to run liberally before relying on one of these repos for an answer that depends on current upstream state.

## Restoring from the manifest (e.g. fresh checkout)

```
mise run reference-repos-setup
```

Clones anything listed in `docs/reference-repos.yaml` that isn't already present under `.reference-repos/`.
