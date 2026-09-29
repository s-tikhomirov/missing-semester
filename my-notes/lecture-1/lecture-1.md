# Lecture 1. Course overview and the shell

Main points from the lecture, plus clarifications from questions while watching.

## Shell and terminal

The shell is a program that reads a command, runs it, and shows the result.
It is the usual language for interacting with the machine in text.

The terminal is the window (a terminal emulator) that handles keyboard and display.
The shell runs _inside_ that window.
Keystrokes go terminal to shell.
Output goes shell to terminal to the screen.

The name "shell" is the outer layer around the OS kernel.

One Linux kernel can host many shells (`bash`, `zsh`, `fish`) because a shell is an ordinary program.
The kernel offers primitives (start a process, open a file).
Each shell is a different language and user interface on top of those primitives.
This course uses bash (or zsh, which accepts most of the same commands).

Why a shell: automate work, combine programs (pipes), and follow how OSS tools are installed and run.

## Prompt, arguments, quoting

Typical prompt: user `@` machine, location (`~` is home), `$` means a normal user (root shows `#`).

The shell splits the line on whitespace.
The first word is the command.
The rest are arguments, passed into the program (Python, Java, `date`, …).

Quote to keep spaces inside one argument: `"hello world"`.
Backslash escapes the next character (treat it as literal).

`cd directory` is enough (`./` is optional for a name in the current directory).

Tab completes.
Double-tab lists matches.

`man` is the full manual.
`--help` or `-h` is the short version.

## PATH, which, and builtins

The shell finds programs by walking `$PATH` (a list of directories) and taking the _first_ match.

`which echo` prints that first path.
`which -a echo` prints every PATH hit.

On this machine `/bin` is a symlink to `/usr/bin`, so `/bin/echo` and `/usr/bin/echo` are the same file listed twice because PATH contains both directories.

A builtin is a command implemented inside the shell process itself.
`type -a echo` shows the builtin first, then the disk copies.
At the prompt, `echo` runs the builtin.

Some builtins exist because they change the shell’s own state (`cd`, `export`, `source`, `exit`).
An external program would run in a child process, and those changes would die with the child.
Others (`echo`, `pwd`) are builtins for speed and so they still work when PATH is empty or `/usr` is missing.
`pwd` means "print working directory" (same as `echo $PWD`).

## Filters and file tools

`ls` lists the current directory, or the path you pass.

`cat` is short for concatenate.
It copies inputs to stdout in order.
With one file and a terminal as stdout, that looks like printing the file.

`sort` prints lines in sorted order (lexicographically by default).
`sort -u` sorts and collapses duplicates.

`uniq` collapses a run of identical neighboring lines.
It only keeps the previous line in memory, so it can stream.
`sort | uniq` groups equals first, then squeezes each group.
`uniq -c` prefixes counts.

`head -n3 file` / `tail -n3 file` print three lines from the start or end.
`-n` is the option (number of lines).
`3` is its argument.
`-n3` and `-n 3` are the same for a one-letter option that takes a value.
`--long-name 3` or `--long-name=3` for long options.
Older `head -3` often still works.

Rule of thumb for glued short options: `-xN` is `-x N` only when `x` is a single letter that expects a value.

`grep PATTERN FILE` is *what*, then *where*.
The pattern is a regex.
`grep -r` searches a directory tree.
`grep -l` lists each filename that has a match (`l` from “list”).
`grep -q` is quiet: it only reports an exit status.

`grep` exit status: 0 if there was a match, 1 if there was none, 2 if something went wrong.
`if grep …` uses that status.
“No match” is a deliberate exit 1 so the shell can treat it as false.

## sed

`sed` is a stream editor: it reads a line, applies a script, writes the result, next line.
Non-interactive means the whole script is given up front.
The script is complete before it starts, so it can sit in a pipe.

Common form:

```console
sed -i 's/pattern/replacement/g' file
```

`-i` is a flag of the `sed` program (edit in place).
`s/pattern/replacement/g` is a tiny program in sed’s language.

The slashes are delimiters around the pattern and replacement, with flags after the third delimiter.
`s` is the command (substitute).
The character right after `s` is the delimiter (any character except backslash and newline).
Then pattern, replacement, then flags.
`g` means every match on the line.
Several flags can follow the third delimiter (`gi`, `gp`, `2g`).

Slash is convention.
Switch delimiter when the text contains slash (`s|a|b|g`, `s#a#b#g`).

## find

`find` walks a tree and keeps entries that match tests.

```console
find ~/Downloads -type f -mtime +30
```

`-mtime` measures modification age in 24-hour periods.
The `+` means greater than.
So `+30` is “mtime more than 30 days ago.”
The unit lives in the option name (`-mtime` days, `-mmin` minutes).

`-atime` is last content read by any process (`cat`, an editor, executing a binary).
`ls` of the directory reads the directory.
Modern Linux often updates atime lazily, so `-mtime` is the reliable clock for “old files.”

```console
find . -name "*.md" -exec grep -l TODO {} \;
```

`{}` is replaced by each path.
`\;` ends the `-exec` command.

`-maxdepth` is a global walk limit (how deep to descend).
Put it after the start path and before tests:

```console
find . -maxdepth 1 -name "*.md"
```

The leading dash is required.
A bare `maxdepth` is another starting path to walk.

`find` is recursive by default.
`-maxdepth 1` limits the walk to the starting directory’s immediate entries.

## Glob versus regex

Both are pattern languages.

The shell expands globs into real paths before the program runs (`*.md`).
Globs know files and directories (`*`, `?`, `[]`).

Programs (`grep`, `sed`) execute regexes against text.
Regex is the richer matching language.
Globs stay in the shell’s path-matching domain.

## awk

`awk` splits each record into fields (`$1`, `$2`, …) and runs a small program.

```console
awk '{print $2}' test.txt
```

prints column 2 of every row.

`-F` sets the field separator (awk’s `FS` variable).
`-F,` means option `F` with argument `,` (comma-separated columns).

`grep` keeps or drops whole lines.
`sed` rewrites the line as a string.
`awk` names columns and can filter (`$2 > 100`), rearrange (`print $3, $1`), accumulate (`sum += $1` plus `END`), or match a regex on one field (`$3 ~ /error/`).
`{print $2}` is already awk’s job: named fields.

`cut` is enough when the job is only extracting a field.

## Pipes and redirects

`|` connects stdout of the left program to stdin of the right.

`>file` sends stdout to `file` (overwrite).
`>>file` appends.
`<file` sets stdin from `file`.

Spaces around `<`, `>`, `|` are optional.
The lecture writes `>file`.
Spaced form is easier to read: `date > thedate.txt`.

fd (file descriptor) is the integer handle for an open stream.
0 stdin, 1 stdout, 2 stderr.
Bare `>` is fd 1.
`2>` rewires stderr.
Glue the digit to the operator: `2>err.txt`.
`2 > err.txt` is the argument `2` plus stdout redirected.

Exercise writeups (including exercise 5, redirects) are in `lecture-1-exercises.md`.

## bash as a language

`if command; then …; else …; fi` runs `command` and uses its exit status (0 means success, `then` runs).

```console
if grep -q 2026 thedate.txt; then echo "it's 2026"; fi
```

`-q` keeps the matching line off the screen.
`if grep 2026 thedate.txt == 1` passes `==` and `1` as extra filenames to `grep`.
`if` uses only the exit status; `-c` prints the count on stdout.
To test the count: `if [ "$(grep -c 2026 thedate.txt)" -eq 1 ]; then …`

`while command; do …; done` repeats until `command` fails (nonzero).
The body keyword is `do` (`then` belongs to `if`).
If `grep` keeps matching, the loop continues.

`for varname in a b c; do …; done`
Often `for i in $(seq 1 10); do …; done` (command substitution).

`[` is the `test` command (usually a builtin; also historically `/usr/bin/[`).
`]` is its required last argument, so it must be its own word (space before `]`).
`[` `"hello"` `=` `"world"` `]` is four arguments after the command name.
`"hello"="world"` glues to one word `hello=world`, and `[` then tests “is this string nonempty?” (true).
Spaces around `=`, `-eq`, `-f` are required so they are separate arguments.

`[[ … ]]` is a bash keyword, parsed like `if` / `then`.
The shell implements the test itself.
`==` is normal there.
Safer with unquoted variables.
It is bash/ksh/zsh.
POSIX `[` / `test` uses `=` for string equality.
`==` inside `[ ]` is a bash extra.

## Scripts

A file of shell commands becomes a script.
The shebang (`#!/bin/bash`) names the program that should interpret the file.
`chmod +x script.sh` marks it executable.
Run it as `./script.sh` so the shell looks in the current directory.
A bare `script.sh` is looked up in `PATH`.

## Spaces in bash (rule of thumb)

The shell splits on unquoted whitespace into words.

- Put a space where a new role starts: command, flag, argument, `[`, `=`, `]`, `then`, `do`.
- Quote (or glue) where it must stay one value: `"hello world"`, `'s/a/b/g'`.
- Shell operators `<`, `>`, `|` may have spaces around them.
- Glue a file descriptor to its operator: `2>`, `&>`.
- One-letter options that take a value may glue: `-n3`, `-F,`.

If the command runs with the wrong meaning, two roles were glued into one word.
If the shell reports `missing ']'` or `unexpected token`, a required word was glued to its neighbor or the wrong keyword was used (`then` vs `do`).