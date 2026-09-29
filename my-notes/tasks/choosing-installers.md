# Choosing how to install CLI tools

Notes from discussing `tldr` and similar “install this extra tool” recommendations.
This is about package managers and install paths in general, not a course lecture.

## What install means

Install puts a program file on disk in a directory that is already on `PATH`, so typing the command name works the same way as typing `date`.
A system install (`apt`) usually lands under `/usr` and is visible to every user.
A user install (`pipx`, `cargo install`, Nix profile) usually lands under your home directory.

The directory you `cd` to when you run the install command does not limit where you can *run* the tool afterward.

## One pages repo, many clients (tldr as example)

The tldr *pages* are shared markdown cheatsheets (MIT).
A *client* is a local program that fetches or caches those pages and prints them (`tldr date`).

On the [tldr Clients wiki](https://github.com/tldr-pages/tldr/wiki/Clients):

- Official means the repo lives under `tldr-pages/` (same maintainers as the pages).
  Examples: Python client (`pipx install tldr`), `tlrc` (their Rust client), an unmaintained C client.
- The original Node client used to live in the main repo; it is still supported, installed via npm.
- Other clients are independent viewers of the same pages.
  Example: `tealdeer` (`tealdeer-rs/tealdeer`), often packaged by distros as `apt install tealdeer`.

Multiple clients exist because the standard is the *data*.
Viewers differ by language, install cost, platform (terminal, browser, editor), and extras (cache, colors, OS-specific pages).

Install docs then list every *store* that already has a package: pipx, apt, Nix (`nixpkgs`), cargo, and so on.
That is one program advertised through many catalogs, not a requirement to use every catalog.

## Isolation (pipx vs Nix vs cargo)

**pipx** isolates a Python import world.
Each app gets its own virtualenv (own `site-packages`).
A launcher on `PATH` (typically `~/.local/bin`) runs that env.
Isolation is per app, not per project folder.

**Nix** isolates the whole dependency graph.
Each package lives at `/nix/store/<hash>-name` with exact inputs.
The binary is wired to those store paths.
A profile is a set of symlinks into the store (`nix profile list`).

**cargo install** isolates at compile time.
Crates are built into the executable.
Runtime is typically one (mostly self-contained) binary in `~/.cargo/bin`.
Sibling cargo-installed tools share that bin directory; they do not share a live crate tree.

| Mechanism | What is private | Shared surface |
|---|---|---|
| pipx | per-app PyPI packages | `~/.local/bin` launchers; a Python runtime |
| Nix | entire closure in the store | the user profile of symlinks |
| cargo install | deps baked into the binary | `~/.cargo/bin` |

## Mixing installers

They all drop files and put a directory on `PATH`.
Tools can call each other as ordinary programs.

The cost is bookkeeping.
Each installer has its own ledger.
Two ledgers can ship the same command name; `PATH` order then picks a winner, and upgrades get confusing.

On this machine Nix profile dirs sit before `/usr/bin`, so a Nix-provided name wins over an apt one with the same name.
Check with `type -a <cmd>`.

## Rule

One command name, one installer.
Decide who owns `tldr` (or `rg`, or `fd`). Only that store installs or upgrades that name.

Walk this list and stop at the first fit:

1. Ubuntu needs it as OS plumbing (base system, packages other `.deb`s depend on, things you want on `apt upgrade`) → `apt`.
   Ledger: `apt list --installed`. Files under `/usr`.
2. Extra CLIs and project pin-downs you already manage with Nix → Nix.
   Ledger: `nix profile list` (or flake / home-manager config).
3. A Python *application* and you want that ecosystem’s app installer → `pipx`.
   Ledger: `pipx list`. Needs Python; pipx calls pip; install pipx first (for example `apt install pipx`).
4. A Rust crate built from crates.io → `cargo install`.
   Ledger: `cargo install --list`.

Language *runtimes* (`python3`, `rustc`) stay on apt or Nix as you already have them.
Apps written in those languages follow steps 2–4.

There is no single OS-wide “all programs” list once stores are mixed.
Use each store’s ledger plus `type -a` for what will actually run.

## Policy for this machine

- apt — Ubuntu base and anything `apt upgrade` should own.
- Nix — extra CLIs (already in use for other projects).
- pipx / cargo — when the tool is missing from apt and Nix, or you want that language’s release cadence.

Example application to tldr: pick **either** `tlrc` from nixpkgs **or** `tealdeer` from apt, and keep that name off the other stores.
