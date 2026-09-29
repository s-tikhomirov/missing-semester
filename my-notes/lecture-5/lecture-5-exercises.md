# Lecture 5 exercises

Notes while working the exercises in `_2026/version-control.md`.
Numbering matches the lecture list (items are numbered `1.` in the source; this file counts them in order).

## Exercise 2. Clone the class website (in progress)

Repository: clone of `missing-semester/missing-semester` in this workspace.

### Last change to `collections:` in `_config.yml`

`git blame _config.yml | grep 'collections:'` points at commit `a88b4eac`.

Commit message for that revision:

```text
Redo lectures as a collection
```

`git show` does not read commit hashes from stdin.
Piping `awk '{print $1}'` into bare `git show` prints HEAD instead.
Pass the hash as an argument, or use `xargs`:

```console
git blame _config.yml | grep 'collections:' | awk '{print $1}' | xargs git show -s
```

`-s` skips the patch; add `--format=%s` for the subject line only.
