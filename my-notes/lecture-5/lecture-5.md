# Lecture 5. Version control (Git)

Clarifications from working through `_2026/version-control.md` and exercises.

## Tools outside core Git

`git-filter-repo` is a separate program (pip or a distro package).
It rewrites history at repository scale.
Git docs often recommend it over older `git filter-branch` for removing large or sensitive files from all commits.

Stock Git ships without a `git graph` subcommand.
The course exercise defines it as a Git alias in `~/.gitconfig` that runs `git log --all --graph --decorate --oneline`.
Editor extensions such as “Git Graph” are separate UIs from the `git` binary.

## Shell aliases vs Git aliases

A shell alias lives in the running shell (usually `~/.bashrc`).
After you edit the file, `source ~/.bashrc` or open a new session so the shell picks up changes.

A Git alias lives in `~/.gitconfig`.
Each `git` invocation reads config from disk, so `git config --global alias.graph …` applies on the next `git graph` with no shell reload.

## Staging by patch (hunk)

`git add -p` (patch mode) walks each change hunk in a file.
Git prompts to stage that hunk, skip it, or split it further.
Stage one logical change, commit, then repeat for other hunks in the same file.
Several commits can each hold part of the same file.

Related: `git add -e` edits the staged diff manually; `git restore -p` unstages or discards by hunk.

## `git bisect`

Binary search over commit history to find where behavior changed (e.g. a regression).

Mark endpoints: `git bisect start`, then `git bisect bad` on a known-broken commit (or current `HEAD`), `git bisect good` on a known-good commit.
Git checks out a middle commit; you test and run `git bisect good` or `git bisect bad` until it names the first bad commit.
`git bisect reset` returns to the branch you started from.

Automate the test with `git bisect run ./script.sh`.
The script runs at each checkout.
Exit code 0 means good, 1–127 means bad, 125 means skip this commit (Git tries another).
Example: build and run tests in the script; Git walks history with the script answering good or bad.

## `git worktree`

Extra working directories for the same repository (same object database, remotes, and refs).
`git worktree add <path> <branch>` checks out another branch in another folder while the main checkout stays as-is.

Typical use: WIP on branch A in the main tree, open a second path on branch B without `git stash` or `git switch` with a dirty tree.
Also useful for two branches at once (compare, test, parallel agents on different branches) with one clone.

Worktrees are peers.
The first checkout is only the “main worktree” in `git worktree list`, not a master copy others follow.
Commits in any worktree update shared refs; uncommitted files stay local to that directory.
Git usually allows one checked-out branch per worktree.

`git worktree list`, `git worktree remove <path>`.
Course mention: agentic coding lecture uses worktrees to isolate parallel agent edits.
