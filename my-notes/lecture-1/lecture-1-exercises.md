# Lecture 1 exercises

Notes while working the exercises in `_2026/course-shell.md`.
Numbering matches the lecture list (all items are numbered `1.` in the markdown source; this file counts them in order).

## Exercise 1. Unix shell

Need a Unix shell (bash or zsh).
`echo $SHELL` should print something like `/bin/bash` or `/usr/bin/zsh`.

## Exercise 2. `ls -l`

`-l` is long listing format.

GNU `man ls` mostly lists flags.
Column meanings live in GNU Info (`info ls`, node “What information is listed”) or the [HTML coreutils manual](https://www.gnu.org/software/coreutils/manual/html_node/What-information-is-listed.html).
`info ls` can crash here while decompressing `/usr/share/info/coreutils.info.gz` (`gzip -d` segfault).
Workaround: `gzip -dc /usr/share/info/coreutils.info.gz | less` and search for that node.

Typical line:

```text
-rw-r--r-- 1 sergei sergei 4096 Sep  8 10:00 notes.md
```

First 10 characters: type plus permission bits.

- Position 1: file type (`-` regular, `d` directory, `l` symlink, …)
- 2–4: owner `r` `w` `x` (or `-` if off)
- 5–7: group
- 8–10: other

Then: hard-link count, owner, group, size in bytes, timestamp, name.

Directory link count is usually 2 plus the number of immediate subdirectories.
Symlink size is the length of the target path string (`bin -> usr/bin` has size 7).
Directory size is the directory record on disk, often a multiple of 4096.
For type `l`, `ls` also prints `->` and the target.

`stat file` labels the same inode fields.

## Exercise 3. Globs

A glob is a pathname pattern the shell expands into real names (`*`, `?`, `[…]`) before the program runs, unless the pattern is quoted.
Brace expansion `{a,b,c}.txt` is a close cousin: it becomes `a.txt` `b.txt` `c.txt`.

`?` is exactly one character (`filea.txt` matches `file?.txt`; `file.txt` and `file10.txt` do not).

Create matching files in a scratch directory, then try `ls *.txt`, `ls file?.txt`, `ls {a,b,c}.txt`.

If nothing matches, default bash passes the glob through as a literal filename.
`ls file?.txt` then becomes `ls` looking for a file named `file?.txt`, hence `cannot access 'file?.txt'`.
`ls` only lists the arguments it received.
The match-or-literal rule is pathname expansion in the shell.

Unquoted `*` `?` `[…]` → shell expands to names.
Quoted `'*.zip'` → the program receives the characters `*.zip`.
`find -name "*.zip"` is quoted so find matches under `~/Downloads`.
Unquoted `*.zip` would expand in the current directory first.

`echo file?.txt` shows what the next command would receive.
`shopt -s failglob` / `nullglob` change the unmatched-glob policy.

## Exercise 4. Quoting

- `'…'` — every character literal (no `$var`, no `\n` as newline).
- `"…"` — `$var` and `$(…)` expand; `!` can trigger history in interactive bash.
- `$'…'` — ANSI-C quotes: C-style escapes (`\n`, `\t`, `\\`, `\'`, …); `$` and `!` stay ordinary characters.

The `$` in `$'…'` turns this mode on (ANSI-C quoting, separate from `$var`).

A string with a literal `$`, a literal `!`, and a newline:

```console
echo $'$!\nnext line'
```

`echo -e` also interprets escapes, but only inside `echo`.
`$'…'` puts the real newline in the word, so any command can receive it.

## Exercise 5. Redirecting stdout and stderr

`ls /nonexistent /tmp` writes the `/tmp` listing on stdout (fd 1) and the missing-path message on stderr (fd 2).

Split them:

```console
ls /nonexistent /tmp > out.txt 2> err.txt
```

Same file, two spellings:

```console
ls /nonexistent /tmp &> out.txt
ls /nonexistent /tmp > out.txt 2>&1
```

`&> out.txt` is a short way of redirecting both output and error into the same file.
`> out.txt 2>&1` is another way of doing the same.

The shell wires every redirection first, then starts `ls`. Writes happen only after that.

Left-to-right setup for `> out.txt 2>&1`:

1. `> out.txt` — point fd 1 at the file (truncate).
2. `2>&1` — point fd 2 at whatever fd 1 is now (same open file, same offset).

In `2>&1`, `&` means the target is a file descriptor.
`2>1` would write stderr into a file named `1`.
`&>` is a different token: `&` glued to `>` as one operator (“both streams”).
A trailing `&` on a command is job control (background).

`2>&1 > out.txt` leaves stderr on the terminal: at `2>&1`, fd 1 is still the tty, so fd 2 copies the tty; then only stdout moves to the file.
Default fds 0, 1, and 2 already point at that tty before any `>` on the line.

Byte order in the file is the order `ls` calls `write()` (error first, then listing).
Wiring order only sets up fds.

Opening the same path twice (`> out.txt 2>> out.txt`) gives two cursors.
Stdout can overwrite stderr in the file.
`2>&1` after `>` shares one open file.

## Exercise 6. Exit status, `&&`, `||`

`$?` is the exit status of the previous command (0 = success).
`&&` runs the next command only if the previous succeeded.
`||` runs the next command only if the previous failed.

Create the directory only when it is missing (`[ -d … ]` fails):

```console
[ -d ./mydir ] || mkdir mydir
```

Remove it only when it exists:

```console
[ -d ./mydir ] && rm -r mydir
```

Same pattern for the lecture path `/tmp/mydir`.
`[ -d ./mydir ]` is `test`; its status is what `||` / `&&` look at.

## Exercise 7. Why `cd` is a builtin

The shell is a process that keeps a working directory.
A process can change only its own cwd (via `chdir`).

`cd` is a builtin: the same ongoing shell process runs its `cd` code
and changes that process’s directory, which is allowed and natural.

An ordinary command is different: the shell forks a child that inherits
the parent’s cwd.
The child may `chdir` and then exit; that cwd change dies with the child.
The parent shell stays in its original directory.

To change the directory of the process you are still talking to,
that process has to `chdir` itself.
The builtin `cd` is that mechanism.

## Exercise 8. Script with `$1` and `[ -f ]`

A script is a file of commands.
`$1` is the first argument to that file when you run it
(`./check.sh file.txt`), empty at an interactive prompt.

Example `my-notes/check.sh`:

```bash
#!/bin/bash

if [ -f "$1" ]
then
	echo "file exists"
else
	echo "file does not exist"
fi
```

The shebang `#!/bin/bash` names the interpreter for `./check.sh`.
`[ -f "$1" ]` is `test`; `-f` means regular file.
Quote `"$1"` so the path stays one word.

Unquoted `[ -f $1 ]` with no argument becomes `[ -f ]`.
`[` then has one argument `-f`, which means “is this string nonempty?”
That is true, so the script prints `file exists` even with no file.
`test` picks its meaning by how many arguments it received
(one word = nonempty string; `-f` plus a path = file test).

A semicolon after `]` ends the test command so `then` can follow on
the same line (`if [ -f "$1" ]; then`).
A newline also ends a command, so `then` on the next line needs no
semicolon.
Spaces around `[`, `-f`, and `]` keep them as separate words.
Indentation does not change meaning.

## Exercise 9. `chmod +x` and `./check.sh`

`./check.sh somefile` runs the file in the current directory
(a bare `check.sh` would be looked up in `PATH`).

Without execute permission the kernel refuses `./check.sh`
(`Permission denied`).
`ls -l check.sh` shows the mode; the `x` bits are off until
`chmod +x check.sh`.
After that, `./check.sh` is allowed to run, and the shebang
selects bash.

`bash check.sh somefile` also works without `+x` because you are
already invoking bash and passing the file as an argument.
`+x` is what makes the file itself a program the kernel will exec.

## Exercise 10. The `set` builtin and `-x`

`set` is a builtin: it changes options of the current bash process.

Lecture “stricter” flags (often `set -euo pipefail`):

- `-e` — exit if a command fails
- `-u` — error on unset variables
- `-o pipefail` — a pipeline fails if any stage fails

`-x` (`xtrace`) prints each command (after expansion) to stderr
before running it, usually with a `+` prefix.
`set +x` turns that trace off.
`help set` lists the others.

`set -x` matches `bash -x script.sh`: minus means that shell option is on.
`set +x` is the opposite sign, used only to clear a `set` option.

`chmod` uses the other plus/minus language: `chmod +x` adds the execute
bit, `chmod -x` removes it.
Same characters, two small languages.

## Exercise 11. Dated backup copy

```console
cp notes.txt "notes-$(date +%Y-%m-%d).txt"
```

`$(…)` is command substitution: the output of that command is spliced
into the word (here the destination name).
`date` and `+%Y-%m-%d` are two words; the space is required.
`date+%Y-%m-%d` would be one command name.
The `+` starts `date`’s format operand (`%Y-%m-%d` is strftime),
not a `set`-style flag.
Quotes keep the whole destination one argument.

## Exercise 12. Flaky-test script and `"$@"`

The lecture script (inline in `_2026/course-shell.md`, copied as
`my-notes/lecture-1/flaky-script.sh`) reruns a test under CPU load
until it fails. A flaky test is one that usually passes and sometimes
fails with the same code.

`stress --cpu 8 &` starts load in the background.
`$!` is the PID of that job; the script `kill`s it at the end.
Install `stress` with `sudo apt install stress`.
If `stress` is missing, the background child exits at once and
`kill $STRESS_PID` reports `No such process`.

`while cargo test my_test …; do` loops while that command succeeds.
`cargo` needs a `Cargo.toml` in the current directory (or a parent).
In `my-notes` there is none, so the first run fails and you get
`Test failed on run 1`.

The exercise: pass the test command as arguments instead of hardcoding
`cargo test my_test`.
`$1` is only the first word (`cargo`).
`$@` is all arguments as separate words (`cargo`, `test`, `my_test`).
Quote `"$@"` so each word stays separate.

Example invocation once the loop uses `"$@"`:

```console
./flaky-script.sh cargo test my_test
```

Run it from a real crate if you want `cargo test` to loop.

## Exercise 13. Five most common extensions

The lecture asks for this in the home directory.
The same pipe was tried on `~/Documents` as a smaller tree.

`find` lists regular files (`-type f`) and walks nested directories.
`-name '*.*'` keeps basenames that contain a `.` (drops `Gemfile`, `CNAME`).
`! -path '*/.*'` drops any path with a hidden component (`.git`, `.gitignore`).

`grep`, `sed`, or `awk` can turn each path into an extension token.
With `sed -E`, two groups split the line at the last dot:

- group 1 `^.*\.` — everything through that dot (greedy `.*` takes the last `.`)
- group 2 `[a-zA-Z0-9]+$` — the suffix (letters and digits, so `mp4` matches)

The replacement is `\2`.
`s///` replaces the whole match; putting `\2` there keeps the extension.

`-n` plus `/p` prints only when the substitution succeeded.
`/p` without `-n` prints matching lines twice and inflates `uniq` counts.

Then count:

- `sort` — identical extensions become adjacent (`uniq` needs that)
- `uniq -c` — count runs
- `sort -g` — numeric order on the count (ascending)
- `tail -n5` — the five largest counts (the lecture’s `head` is the other end if you `sort -nr`)

```console
find ~/Documents/ -type f -name "*.*" ! -path "*/.*" \
  | sed -n -E 's/(^.*\.)([a-zA-Z0-9]+)$/\2/p' \
  | sort \
  | uniq -c \
  | sort -g \
  | tail -n5
```

`grep` on this pipe searches the path strings on stdin, not file contents.
Default `grep` prints the whole matching line.
`-o` prints only the matching substring.

The pattern must be the extension itself.
`'\.[a-zA-Z0-9]+$'` is the last `.` plus suffix at end of line.
`^.*\.…$` would match the entire path, so `-o` would print the entire path.

Plain `grep` uses basic regex, where `+` is a literal character.
`-E` makes `+` mean “one or more,” as in `sed -E`.

```console
find ~/Documents/ -type f -name "*.*" ! -path "*/.*" \
  | grep -o -E '\.[a-zA-Z0-9]+$' \
  | sort \
  | uniq -c \
  | sort -g \
  | tail -n5
```

Tokens look like `.md` (the dot is in the match).
`sed`’s `\2` was only `md`.
Counts are the same if every token keeps that leading dot.

`awk -F.` splits each path on `.`.
`$1`, `$2`, … are fields; `NF` is how many fields there are; `$NF` is the last field (the extension).

`NF > 1` means “there was at least one dot.”
`$NF > 1` is different: it takes the **text** of the last field and asks whether that value is numerically greater than 1 (`md` becomes 0, so almost every extension is dropped).

```console
find ~/Documents/ -type f -name "*.*" ! -path "*/.*" \
  | awk -F. 'NF > 1 { print $NF }' \
  | sort \
  | uniq -c \
  | sort -g \
  | tail -n5
```

Tokens are like `md` (no leading dot), same as the `sed` replacement.
`awk` does not limit the suffix to `[a-zA-Z0-9]`; the last field is whatever follows the last `.`.

For the home directory, use `~` in place of `~/Documents/`.
A hidden-name skip is optional; the lecture does not require it.

## Exercise 14. `find` and `xargs` with `wc -l`

`find` writes paths on stdout.
`wc -l file.sh` counts lines in a file named as an argument.
With no filenames, `wc` counts stdin.

`find . -name '*.sh' | wc -l` therefore counts how many `.sh` paths were printed, not lines inside those files.

`xargs` reads that name stream and runs a command with the names on `argv`.
There is no pipe between `xargs` and `wc`: `wc` is the program `xargs` starts, not the next pipeline stage.

Bare `xargs` runs `echo` with those words (names on one line).
`xargs wc -l` builds the equivalent of `wc -l ./a.sh ./b.sh`.

`find -exec wc -l {} +` also passes paths as arguments.
The exercise asks for `xargs` instead.

Default `find` separates paths with newline.
Default `xargs` splits on any whitespace and treats quotes specially, so `my script.sh` becomes two arguments.

A path cannot contain a NUL byte.
`-print0` makes `find` end each path with NUL.
`xargs -0` splits only on NUL and takes each chunk as one argument.
`-0` is an option of `xargs`, before the command name.

```console
find . -type f -name "*.sh" -print0 | xargs -0 wc -l
```



## Exercise 15. `curl` and lecture count

`curl` writes the page HTML to stdout.
`-s` on `curl` silences the progress meter (stderr); the HTML in the pipe is the same either way.
`grep -s` is a different flag (suppress errors about missing files).

`less` is not part of the solution.
It is a pager so you can read the HTML and pick a pattern.
Once you have the pattern, the pipe is `curl | grep`.

`grep -c` prints the number of matching lines, not the matching text.

The 2026 syllabus entries are links whose `href` starts with `/2026`.
A line-oriented count:

```console
curl -s https://missing.csail.mit.edu/ | grep -c 'href="/2026'
```

That pattern also matches the nav link `<a href="/2026/">lectures</a>`, so the count is one higher than the syllabus list (nine lectures plus nav).
A tighter pattern (for example a path after `/2026/` other than the bare year index) would drop the nav.

Older-year lectures on the same page use `/2020/` and `/2019/`, so this command counts only 2026 URLs.

## Exercise 16. `jq` names with version greater than 6

`curl` fetches JSON on stdout.
`jq .` is the identity filter: pretty-print the current value (the whole document).
That shows a JSON array of person objects, each with `name`, `version`, and other keys.

In `jq`, `.` always means “the value we are looking at now.”
At the start of the filter, that is the whole document.

`.[]` iterates that value:
for an array, it streams each element as the new current value (one person object at a time).
It is not an empty array literal.
`[]` on its own would be an empty array.

The `|` inside the `jq` program is `jq`’s own pipe: the output of one filter is the input of the next.
It is separate from the shell `|` between `curl` and `jq`.

`select(.version > 6)` keeps a person when that object’s `version` field is greater than 6.
`.name` then picks the name string from the objects that survived.

```console
curl -s https://microsoftedge.github.io/Demos/json-dummy-data/64KB.json \
  | jq '.[] | select(.version > 6) | .name'
```

## Exercise 17. `awk` filter and swap columns

Shell `printf` follows C `printf`: a format string plus arguments, with `\n` as newline.
`printf 'a 50 x\nb 150 y\nc 200 z\n'` writes three test lines to stdout (same text as C `printf("a 50 x\nb 150 y\nc 200 z\n")`).

`awk` splits on whitespace into `$1`, `$2`, `$3`.
`$2 > 100` is a numeric pattern: only those lines run the action.
`print` writes fields separated by OFS (a space by default).

Swap first and third, keep the middle column:

```console
printf 'a 50 x\nb 150 y\nc 200 z\n' | awk '$2 > 100 { print $3, $2, $1 }'
```

That prints `y 150 b` and `z 200 c`.
`print $3, $1` drops `$2`, so the lines become `y b` and `z c`.
The lecture’s example `$3 ~ /pattern/` is a regex on a field; `>` is a number compare.

## Exercise 18. SSH log pipeline and most-used commands

Lecture pipeline (top SSH users, comma-separated):

```console
ssh myserver 'journalctl -u sshd -b-1 | grep "Disconnected from"' \
  | sed -E 's/.*Disconnected from .* user (.*) [^ ]+ port.*/\1/' \
  | sort | uniq -c \
  | sort -nk1,1 | tail -n10 \
  | awk '{print $2}' | paste -sd,
```

- `ssh … '…'` — run the quoted pipeline on the remote host; its stdout comes back locally
- `journalctl -u sshd -b-1` — sshd logs from the previous boot
- `grep "Disconnected from"` — keep disconnect lines
- `sed -E` — capture the username, same whole-line-plus-group idea as exercise 13
- `sort | uniq -c` — count identical names
- `sort -nk1,1 | tail -n10` — ten highest counts (`-n` numeric, `-k1,1` only the count field)
- `awk '{print $2}'` — drop the counts, keep the names
- `paste -sd,` — join those names with commas on one line

Most-used local commands, counting only the first word of each history line (stages after `|` are ignored).
The `history` builtin prints an event number, then the line, so the command token is `$2`.

```console
history | awk '{ print $2 }' | sort | uniq -c | sort -g | tail -n5
```

`~/.bash_history` has no event numbers; there the command would be `$1`.
`history` is the in-memory list; the file may lag until the shell exits (or `history -a`).
zsh timestamps in `~/.zsh_history` need a different field.

