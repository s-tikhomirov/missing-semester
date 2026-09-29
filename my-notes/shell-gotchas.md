# Shell and terminal gotchas

Things that look similar in the terminal but mean different things depending on quoting, context, or which program parses them.
This file is not tied to a single lecture.

## Single quotes and double quotes

In the shell they are not interchangeable in general.

The shell removes the quotes before the program runs.
`grep` and `echo` never see the quote characters.
What they see is the resulting argument after quoting rules have already applied.

### When they behave the same

For a string with no characters the shell would expand or interpret, both forms produce the same argument.

```shell
echo hello
echo 'hello'
echo "hello"
```

That includes many literal words: `ls`, filenames without `$` or spaces, and patterns that contain no `$`, backticks, or `!`.

### When they differ

Single quotes (`'...'`) are literal.
The shell does not expand variables, does not run command substitution, and does not process escape sequences inside them.

```shell
foo=bar
echo '$foo'
# prints $foo
```

Double quotes (`"..."`) still group the text into one argument (spaces stay inside),
but the shell does expand `$var`, `${var}`, `$(command)`, and `` `command` ``,
and it honors some backslash escapes (`\$`, `` \` ``, `\"`, `\\`).

```shell
foo=bar
echo "$foo"
# prints bar
```

That is why `echo $files` and `echo "$files"` are different,
and why `'echo "$files"'` vs `"echo '$files'"` differ again if you nest them.

A single quote cannot appear inside single quotes without ending the quoted span.
A double quote can appear inside single quotes as a literal `"`.

### Other contexts where the same characters mean something else

Quotes are also special outside “this bash command line,” with different rules:

- In JSON, keys and strings use double quotes.
  A common pattern is to wrap the whole JSON in single quotes so the *shell* does not expand `$` inside it: `'{"n": 1}'`.
- In Python and JavaScript, both quote styles make strings, with only small differences (escaping, interpolation in some languages).
- In SQL, single quotes usually mean a string literal and double quotes usually mean an identifier.
- In `ssh host 'echo $HOME'` the local shell does not expand `$HOME`; in `ssh host "echo $HOME"` it does, before SSH runs.

When something “does not work with quotes,” first ask which layer is parsing them: the local shell, a remote shell, or the program’s own language.

## Globs and regular expressions

`*` does not mean “anything” by itself in every tool.
The same character is two different languages.
The first question is who is parsing the pattern.

A glob is a filename pattern.
The shell expands it into a list of matching names *before* the program starts
(`ls *.txt` becomes `ls test.txt notes.txt`).
`find -name`, `gitignore`, and some installers also speak globs.

A regular expression (regex) is a pattern over text, used by `grep`, `sed`, `awk`, many editors, and language libraries.
Those programs receive the pattern as an argument; the shell must not eat it first, which is why you often quote it.

### Star and dot star

In a glob, `*` already means “any string, including empty.”
There is no need for a dot in front.

```shell
ls *.txt
# names that end with .txt
# the * ate the prefix; the . is a literal dot in the filename
```

In regex, `*` means “the previous piece, zero or more times.”
It does not mean “any character.”
The piece that means “any character except newline” is `.`.
Putting them together, `.*` means “any character, any number of times.”

```shell
echo 'hello' | grep '.*'
# matches, because . * is “anything”
echo 'hello' | grep '*'
# does not mean “match anything”; * has nothing to repeat
```

So `*` in `ls *` and `.*` in `grep '.*'` are the two ways people say “everything,” in two languages.

The `.` is also different:

- Glob `.` is a literal dot (`ls .gitignore`, `ls *.txt`).
- Regex `.` is a wildcard for one character.
  To match a real dot in grep, escape it: `grep '\.txt'`.

### Other characters that swap meaning

| Character | Glob (filenames) | Regex (text) |
| --- | --- | --- |
| `*` | any string | repeat the previous piece, 0 or more |
| `?` | any one character | previous piece optional (0 or 1) |
| `.` | literal `.` | any one character |
| `[abc]` | one character from the set (similar idea in both) | same idea, with more syntax |

Brace lists `{a,b,c}` are neither glob nor regex.
They are brace expansion in the shell (`touch {foo,bar}.txt`).

`find` uses globs for `-name` and regex for `-regex`:

```shell
find . -name '*.txt'
find . -regex '.*\.txt'
```

Same intent, two pattern languages.
The quotes stop the shell from globbing `*.txt` in the current directory before `find` sees it.

### A few side-by-side examples

```shell
# Glob: files whose names end in .py
ls *.py

# Regex: lines that contain .py as text with a real dot
grep '\.py' file.txt

# Glob: any single-character name
ls ?

# Regex: the previous letter is optional (needs extended regex, see below)
grep -E 'gre?p' file.txt

# Regex “anything, then foo, then anything” on each line
grep '.*foo.*' file.txt

# Glob “anything, then foo, then anything” as a filename
ls *foo*
```

If a `*` or `?` “does the wrong thing,” check the layer:
the shell expanding names, or `grep`/`sed` matching line text.

## Basic regex and extended regex

Regex is not one language.
Unix tools ship two POSIX dialects: basic regular expressions (BRE) and extended regular expressions (ERE).
The metacharacters you type change between them.
A third family, Perl-compatible regex (PCRE), shows up in `grep -P`, Python, JavaScript, and ripgrep.

`.`, `*`, `^`, `$`, and `[...]` mean the same in BRE and ERE.
The split is about `?`, `+`, `|`, `{n,m}`, and `(...)`.

### What is special without a backslash

In BRE (plain `grep`, plain `sed`), those extras are *literal* unless you escape them.
Grouping and “or” look like `\(...\)` and `\|` (the `\|` form is a GNU extension).
“One or more” is often written `[0-9][0-9]*`, or GNU `grep '[0-9]\+'`.

In ERE (`grep -E`, `egrep`, `sed -E`, `awk`), those extras are special *without* a backslash.
You escape them only when you want a literal `+`, `?`, or `|`.

```shell
# “foo or bar”
grep 'foo\|bar' file.txt      # BRE (GNU grep)
grep -E 'foo|bar' file.txt    # ERE

# one or more digits
grep '[0-9][0-9]*' file.txt   # BRE, portable
grep -E '[0-9]+' file.txt     # ERE

# optional u in colour/color
grep 'colou\?r' file.txt      # BRE (GNU)
grep -E 'colou?r' file.txt    # ERE

# a capture group
sed 's/\(foo\)/\1-bar/'       # BRE
sed -E 's/(foo)/\1-bar/'      # ERE
```

Default `grep 'gre?p'` searches for the four characters `g`, `r`, `e`, `?`, `p`.
`grep -E 'gre?p'` treats `?` as “optional previous character.”

### Which to use

For new interactive `grep`/`sed` work, ERE (`-E`) is the usual choice: patterns read like the regex you see in tutorials.

Use BRE when you are matching an old script, a POSIX `sed` without `-E`, or a example that writes `\(` and `\+`.

`awk` is ERE already; it has no BRE flag.
`grep -P` and `rg` are not POSIX ERE; they add lookaround, `\d`, lazy `*?`, and so on.
Python `re` is in that camp, not BRE.

It matters the moment you type `+`, `?`, `|`, or `(...)`.
If the match is silently missing, check whether the tool is in BRE or ERE before changing the pattern.

## Spaces in shell commands and scripts

A space is how the shell splits words.
It is not decoration, and it is not Python-style layout.
Whether a space is required, forbidden, or optional depends on whether the next token must be a new word.

### Required

A command name and its arguments are separate words:

```shell
ls -l
[ "$a" = "$b" ]
[[ "$a" == "$b" ]]
if [ -f "$f" ]; then echo yes; fi
{ echo hi; }
```

`[` is a command (like `test`).
`=` and `]` are arguments, so they need spaces around them.
`[[` and `]]` are reserved words and must stand alone.
`{` is a reserved word; there must be a space (or newline) after `{`.

### Forbidden

Assignment glues name, `=`, and value into one word:

```shell
foo=bar
# foo = bar  runs a command named foo with arguments = and bar
```

Brace expansion cannot have spaces inside the braces: `{foo,bar}.txt`, not `{foo, bar}.txt`.

File-descriptor redirects glue the number to the operator: `2>`, `2>>`, `2>&1`.
`2 > file` redirects stdout and passes `2` as an argument.

### Optional (style)

Spaces around `|`, `&&`, `||`, and around `>` / `<` (with no fd number) do not change meaning:

```shell
ls | grep txt
echo hi > out.txt
```

Inside `$(( ... ))` and `(( ... ))`, spaces around operators are optional.
Inside `"$foo"` vs `" $foo "`, a space is data, not syntax: it becomes part of the string.

### Rule of thumb

Treat a space as a word break.

1. Put a space between things that must be separate words (command, flags, `[` `]`, `[[` `]]`, `{`, `then`, `do`).
2. Put no space inside a single token (`foo=bar`, `2>`, `{a,b}`, `$foo`, `"$foo"`).
3. Use spaces around `|` and `>` for reading; they are optional there.

If you are unsure, extra spaces are safer around tests and keywords, and extra spaces are unsafe around `=` in assignment and inside `{...}` braces.
If a value contains spaces, quote it (`"$name"`) so the space stays inside one word instead of splitting the line.

## Signals that ask a program to stop

Stopping a process is not one thing.
The kernel delivers a *signal*; some are requests the program can handle, some are not.

Less aggressive signals (for example `SIGINT` from Ctrl-C, `SIGTERM` from a normal `kill`) mean “please exit.”
The program may install a handler, finish in-flight work, and then quit.

A usual pattern in that handler is to clean up so the process leaves the system consistent:
delete or finish temp files, close a partial download, unlock a file, flush logs.
Otherwise a crash mid-write can leave a corrupt file behind.

More aggressive signals cannot be used that way.
`SIGKILL` (`kill -9`) is not delivered to user code; the process is torn down.
No handler runs, so no cleanup.

Prefer a catchable stop first (`kill PID` / `SIGTERM`).
Use `SIGKILL` when the program ignores the request or is stuck.
The earlier points still apply: a pipeline can also die from `SIGPIPE` when a later stage has already exited.

## Numbered things dollar vs percent

A number with a sigil in front is “the Nth X,” but the sigil depends on which language is parsing it.
`$` and `%` are not interchangeable.

In the shell’s job control, `%` names a **job** from `jobs`:

```shell
jobs          # [1] sleep 1000
kill %1       # job number 1
fg %2
```

`%` here is not “percent of.” It is the job prefix. `+` / `-` on the `jobs` list pick the default job; they are not `%+` unless you write that form (`%%` or `%+` is the current job).

`$` in the shell names **parameters**, not jobs:

- `$1`, `$2` are positional arguments to the script or function (not job 1).
- `$!` is the pid of the last background process (a process id, not `%n`).
- `$$` is the shell’s own pid.
- `$?` is the last exit status.

In `awk` (and often `sed`), `$` names **fields of the current line**:

```shell
echo 'a b c' | awk '{ print $2 }'
# prints b
```

`$2` there is the second whitespace-separated field, not a shell argument and not job 2.
The shell must not expand it first, which is why the awk program is quoted: `'{ print $2 }'`.

Same digits, three namespaces: job `%1`, shell argument `$1`, awk field `$2`.
Ask which program is reading the `$` or `%` before treating them as the same “Nth thing.”

## Interactive, login, and startup files

Two independent axes describe each bash process: interactive vs non-interactive, and login vs non-login.
The terminal emulator is a pty window.
Interactivity belongs to whichever shell PID is running.

An open tab usually has one long-lived interactive bash (prompt, reads your lines).
Running `./script.sh` starts a child bash to execute the file; that child is non-interactive.
The parent stays interactive while it waits.
When the script exits, the same parent shows the prompt again.
The script file has no interactivity flag; only the shell instance running it does.

Non-interactive shells do not have to come from an interactive parent.
Cron, CI, `ssh host 'cmd'`, `make`, and subprocesses can start non-interactive bash with no terminal session above them.

Login is how bash was started (login flag, `bash -l`, typical `ssh` session shell), which picks profile startup files.
It is separate from “did I type a password recently.”
Local GUI tabs (Tilix, Cursor, tmux panes) are usually interactive non-login.
You already have a desktop session from GDM, so the emulator starts bash without the login convention.

Typical bash startup on Linux:

- Login interactive → `~/.bash_profile` (or `~/.profile`)
- Non-login interactive → `~/.bashrc`
- Non-interactive → minimal startup; full `.bashrc` often skipped

Many people `source ~/.bashrc` from `.bash_profile` so SSH login still gets aliases.
Check login with `shopt login_shell`; interactive bash has `i` in `$-`.
