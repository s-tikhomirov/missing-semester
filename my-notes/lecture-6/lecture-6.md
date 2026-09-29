# Lecture 6. Packaging and shipping code

Clarifications from `_2026/shipping-code.md` and chat while watching the lecture.

## Dependency install order

Installers resolve the full tree (direct plus transitive deps).

Install respects edges in the graph: prerequisites before dependents (topological order).

Among sibling packages with no edge between them, any order works.

Tools may pick different valid orders or install in parallel when safe.

## One version per name in one environment

`import urllib3` looks up one module name on one search path.

Two versions of the same distribution name share the same install slots.

The resolver picks one version per name for that venv, or reports a conflict.

Separate venvs, containers, or machines each hold their own resolved set.

Rust can link multiple crate versions in one binary; default pip layout uses one winner per name in one env.

## pip, pip3, python, python3

On this machine `pip` and `pip3` both target Python 3.12 from the distro.

`pip3` habit comes from the Python 2 / Python 3 split on older Linux.

`python3 -m pip` ties the installer to a specific interpreter.

Linux docs and scripts often say `python3`; `python` may be missing or symlinked via `python-is-python3`.

Inside an activated venv, `python` points at the venv interpreter.

## uv vs pip

pip ships with Python; ubiquitous in tutorials and CI.

uv is a fast Rust tool from Astral: resolve, lock, sync, venv, and `uv pip` as a pip-shaped CLI.

uv fits lockfile-first workflows; pip remains the universal default.

Rough analogy: uv bundles much of the split Python workflow (deps, env, lock) like Cargo does for Rust.

Cargo is the official Rust toolchain including compile; uv orchestrates install and env while CPython still interprets `.py`.

Python stays spec-driven (`pyproject.toml`); Poetry, Hatch, and pip coexist.

## Why uv is fast

Native Rust binary, modern resolver, parallel download and install, global cache with hardlinks.

Large warm-cache syncs can be an order of magnitude faster than pip; tiny installs are network-bound.

## Wheels and sdist

Wheel (`.whl`): zip with layout plus metadata; `pip` / `uv` unpack into `site-packages`.

Extension wheels ship compiled `.so` / `.pyd` for that platform.

Pure Python wheels (`py3-none-any`) still ship `.py` source; install skips a local build step.

sdist triggers the build back-end on install when no matching wheel exists.

## venv

Directory with interpreter links, `bin/`, and `lib/.../site-packages/`.

`source .venv/bin/activate` prepends `.venv/bin` to `PATH` so `python` and `pip` hit this env first.

## pyproject.toml, setup.py, requirements.txt

`pyproject.toml`: modern manifest plus `[build-system]` back-end (PEP 518 / 621).

`setup.py`: setuptools-era imperative config; often migrated into `pyproject.toml`.

`requirements.txt`: flat list for `pip install -r`; optional alongside lockfiles.

One project usually centers on `pyproject.toml` plus a lockfile (`uv.lock`, etc.).

## Install front-end vs build back-end

Front-end: what you run (`pip`, `uv`, `python -m build`, `uv build`).

Back-end: hatchling, setuptools, flit — builds wheel/sdist from the tree via PEP 517.

Build means assemble a distributable artifact; run time still interprets Python source.

## Developer vs user

Developer: author `pyproject.toml` → `uv build` or `python -m build` → `dist/*.whl` → publish (PyPI or share file).

User: venv → `pip install pkg` or `uv sync` → import or CLI.

User runs install front-end only; back-end runs when install must build from source.

Prebuilt wheel on PyPI: user side is mostly download and unpack.

## uv pip install vs uv sync

`uv sync`: install locked deps for the current project.

`uv pip install`: pip-compatible ad hoc installs and `requirements.txt`.

## Typer

CLI library from type hints; built on Click.

Common in tutorials; argparse is stdlib; Click is widespread in older CLIs.

## Lock files

One lock file per project repo (`uv.lock`, etc.), usually committed.

Records exact resolved versions for that tree only.

`uv sync` installs into the project venv from that file.

Download cache (`pip` / `uv` cache) is per user or machine for speed; separate from what the lock pins.

## Compatible release (`~=`)

PEP 440: `~=2.1.0` means `>=2.1.0` on the same major.minor line (`2.1.*`, below `2.2.0`).

`~=2.1` (two parts) allows any `2.x` from `2.1` upward.

Patch-only window for three-part form; minor bumps need a wider spec.

npm/cargo `^2.1.0` follows semver shape and allows `2.2.0` within major `2`.

## Docker run and images

`docker run` starts a container from a local image tag.

Missing image triggers a registry pull first; pull time dominates a cold host.

Repeat runs reuse local layers and feel instant.

## Container vs VM

Container processes use the host Linux kernel with namespaces and cgroups.

Image supplies root filesystem; host schedules CPU and RAM.

VM runs a guest OS with its own kernel on a hypervisor; boot and isolation boundary differ.

## Docker vs venv (mental model)

Both target reproducible environments.

venv scopes Python packages on the host interpreter.

Container scopes a full filesystem slice (distro libs, any language, tools).

## Image layers

Each Dockerfile step that changes the filesystem adds a read-only layer (content-addressed delta).

Images stack layers; containers add one writable layer on top.

Shared base layers dedupe across images; build cache skips unchanged steps.

## Dockerfile, image, container

Dockerfile: versioned build recipe in the repo.

`docker build` produces an image (layer stack plus metadata).

`docker run` creates a container (image plus writable layer plus process).

## Compose vs Dockerfile

`docker-compose.yml` declares the whole stack (services, ports, env, volumes, `depends_on`).

Per service: `build: .` runs `docker build` (default `Dockerfile` in context).

Per service: `image: redis:7-alpine` pulls and runs; no local Dockerfile for that role.

`docker compose up` builds only services with `build:`; cache unless `--build` or context changed.

## Compose service

A service is a named role in YAML (`web`, `cache`, …).

Default `up` is one container per service.

`--scale web=3` runs three containers from the same service definition.

Internal DNS resolves the service name to replica backends.

## Visit counter architecture

Stateless `web` handles HTTP; count lives in a separate service (lecture video: etcd; notes: Redis).

Restart or scale web without losing the total when all replicas share one store.

Compose wires hostname (`cache` / `etcd`), env URL, and `depends_on`.

## Persistence

Process memory resets when the process stops.

Same container stop/start keeps its writable filesystem layer.

New container from the image starts from image layers only unless a volume or external store holds data.

Volumes, bind mounts, and DB/KV containers back lecture-style durability.

## Horizontal scale

Replicas coordinate through shared state (etcd, Redis, SQL), each request via any replica.

Host port `8080:8080` binds one container; multiple web replicas often need a reverse proxy service.

## Reverse proxy

Forward proxy: client configures outbound exit (corporate gateway).

Reverse proxy: public inbound front (nginx, caddy) to app backends; lecture scaling pattern.

## Host boot and systemd

`/etc`: host configuration tree (many files generated or edited on Ubuntu).

systemd: PID 1, starts units; `systemctl` controls and enables them.

Lecture unit: `WorkingDirectory` plus `ExecStart=docker compose up -d`.

`Requires=docker.service` pulls docker into the dependency graph.

`After=docker.service` orders start so dockerd is up before compose.

`WantedBy=multi-user.target` plus `enable` ties the stack to boot.

## Nix

See `nix.md`.
