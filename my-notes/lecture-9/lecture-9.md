# Lecture 9. Code quality

Clarifications from `_2026/code-quality.md` (pre-commit section).

## Git hooks (core)

Git ships with hooks.
They are small programs Git runs at fixed points (`pre-commit`, `pre-push`, and others).
Default location: `.git/hooks/`.

That directory sits inside `.git`.
A normal clone does not share hook scripts through the tracked tree unless you add extra wiring.

A bare hook is usually one script you maintain.
You invoke formatters, linters, or tests yourself.
You install dependencies, pin versions, and handle failures on each machine.

## pre-commit framework

The [pre-commit](https://pre-commit.com/) project is a separate tool.
It uses Git’s `pre-commit` hook point.
`pre-commit install` registers a thin hook that delegates to the framework.

The lecture line means: Git already exposes the hook; the framework makes team-wide, multi-tool checks practical to define, version, and reproduce.

What the framework adds:

| Piece | Role |
| --- | --- |
| `.pre-commit-config.yaml` | Committed config; same checks after `pre-commit install` on each clone |
| Hook catalog | Ready-made hooks for many formatters and linters; list repos and revisions instead of glue scripts |
| Isolated environments | Fetch and run each hook with pinned versions in its own environment |
| Staged files | Typical setup runs only on what you are committing |
| One entry point | One YAML file can orchestrate Python, JS, shell, and other tools in one pre-commit step |

Projects often run formatters and linters (sometimes tests) before every commit so committed code matches style and passes basic checks.
