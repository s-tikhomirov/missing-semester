# Lecture 2 exercises

Notes while working the exercises in `_2026/command-line-environment.md`.
Numbering matches the lecture list (items are numbered `1.` in the source; this file counts them in order).

## Exercise 1. End of options (`--`)

POSIX-style tools treat words that start with `-` as flags unless you mark the end of the option list.
`--` means “no more flags”; everything after it is a positional operand, even if it looks like `-something`.

Example from the lecture: `touch -- -myfile` creates a file whose name is literally `-myfile`.
The same issue appears when deleting that file with `rm`.

Leading-dash filenames are rare in everyday naming, but the pattern matters for archives, scripted paths, and pipelines that pass paths into `rm`, `cp`, `mv`, and similar tools.
The exercise highlights the escape hatch for operands that look like flags.

## Exercise 2. `ls` flags

Command:

```console
ls -alht --color
```

- `-a` — all entries, including names that start with `.`
- `-l` — long listing (permissions, owner, size, time, name)
- `-h` — human-readable sizes with `K`, `M`, and so on (with `-l`)
- `-t` — sort by modification time, newest first
- `--color=auto` — colorized output

## Exercise 3. Process substitution and `printenv` vs `export`

```console
diff <(printenv | sort) <(export | sort)
```

Process substitution gives `diff` two readable paths so it can compare two command outputs without manual temp files.
After sorting, the two lists cover the same exported environment, but every line differs in spelling: `printenv` prints `NAME=value`, while bare `export` prints bash `declare -x NAME="value"` lines.
`diff` works line by line, so it marks whole lines as changed even when only the format differs.

## Exercise 4. `marco` and `polo`

Solution: `my-notes/lecture-2/exercises/marcopolo.sh`.

Define two bash functions in a file and load them with `source marcopolo.sh` so they run in the current shell.
`marco` stores `$(pwd)` in a shell variable (`MARCO_PWD`).
`polo` runs `cd` to that stored path from any directory.
Functions can change the interactive working directory; executing the same file as `./marcopolo.sh` would start a child shell and could not leave `cd` behind.

## Exercise 5. Return codes

Solution: `my-notes/lecture-2/exercises/return-codes-1/run_failing_script.sh`.

## Exercise 6. `pgrep` and `pkill` (signals and job control)

After `sleep 10000`, Ctrl-Z, and `bg`, the job keeps running as a background process while the shell stays interactive.
`pgrep -lf "sleep 10000"` lists the matching PID and process name; `-f` matches the full command line and `-l` is for `pgrep` only (not `pkill`).
`pkill sleep` sends the default signal (`SIGTERM`) to processes whose name matches `sleep`, so you can stop the job without typing the PID.

## Exercise 7. `wait` and `pidwait`

Solution: `my-notes/lecture-2/exercises/return-codes-2/pidwait.sh` (define `pidwait`, then `source` the file).

`wait` with no arguments blocks until every background child started in this shell has exited; it does not apply to arbitrary PIDs from another session.
`pidwait` takes a PID and loops: `kill -0` probes existence via exit status (signal 0 does not kill), `sleep` avoids a busy loop, and the function returns when the process is gone.
Test with `sleep 10 &` then `pidwait $!`, not `$(sleep 10 &)`, because command substitution captures stdout, not the background job’s PID.

## Exercise 8. Files and Permissions

Skipped.

## Exercise 9. Terminal Multiplexers

