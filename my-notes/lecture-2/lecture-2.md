# Lecture 2. Command-line environment

Main points that were unclear, plus clarifications from questions. Longer notes on quoting, globs, regex dialects, and spaces live in `my-notes/shell-gotchas.md`.

## Brace expansion

Lists like `{foo,bar,baz}.txt` are brace expansion in the shell.
Spaces inside the braces break it.
`{foo, bar, baz}.txt` is three words.

## Pipes and streams

A pipe does not mean “first program finishes, then the next starts.”
Programs in a pipeline run together.
The left side starts writing a stream; the right side already reads it.

A job in the middle does not wait for the whole stream to end before it starts.
If there is no data yet and the writer is still open, `read` blocks.
The process stays alive.
End-of-file is different: the writer closed stdout, then `read` returns 0 and a typical filter exits.

If a later job exits first, an earlier writer can get `SIGPIPE`.

`grep PATTERN` with no file reads stdin.
A pipe already connects stdout of the left job to that stdin, so `-` is optional.
`-` is the explicit “this argument is stdin” marker when a program needs a filename slot filled, or when mixing files and stdin.

## Variables, quotes, and grep

`files=$(ls)` keeps the newlines from `ls`.
`echo $files` splits on whitespace and `echo` reprints the pieces separated by spaces, so you see one line.
`echo "$files"` keeps the newlines.
The lecture uses the quoted form so `grep` sees one name per line.

`'` and `"` are not interchangeable.
Single quotes are literal.
Double quotes still expand `$var` and `$(...)` while keeping spaces as one argument.

Default `grep` is POSIX basic regex.
`\d` is not a digit there; use `[0-9]` (or `grep -P '\d'`).
`grep -q` prints nothing and only sets the exit status (0 if a match, 1 if not), which is what `if` needs.

## Environment vs arguments

`TZ=Asia/Tokyo date` is a one-shot assignment in front of a command.
`TZ=Asia/Tokyo` is one word (no spaces around `=`).
`date` is the program.
Only that child sees `TZ`.
`Asia/Tokyo` is an IANA zone id under `/usr/share/zoneinfo/`.

Arguments are that invocation’s `argv` (files, flags, patterns).
The environment is a key/value map the process can look up by name (`getenv`, `os.environ`).
`DEBUG=1 ./program` is not an argument.
The program only changes behavior if it reads `DEBUG`.

`export` means “put this shell variable on the environment children inherit.”
A plain `DEBUG=1` stays in this shell.
`export DEBUG=1` is visible to later commands.
Prefix assignment is export for one command only.

## Signals

Stopping a process is a signal.

`SIGINT` (Ctrl-C) and `SIGTERM` (`kill PID`) are requests.
A handler can delete temp files and leave a consistent state, then exit.
`SIGKILL` (`kill -9`) is not delivered to user code, so that cleanup cannot run.

## Jobs, foreground, and background

A job is what the shell tracks as one unit: one command, or a whole pipeline (several programs).
Job numbers (`%1`) name that unit.
A pid names one process.
`jobs -l` shows both.
`+` is the default job for `fg` / `bg` with no number.
`-` is the previous default.”
Those marks are not the same as the `[1]` `[2]` `[3]` ids.

Stopped in `jobs` means paused.
Ctrl-C asks the foreground job to quit (`SIGINT`).
Ctrl-Z pauses the foreground job (`SIGTSTP`) and gives the prompt back.

Having the prompt does not mean the job is running in the background.
Foreground means the job owns the terminal (no prompt).
Background means it runs without owning the terminal (prompt is yours).
Stopped means it is not executing.
A stopped job is not a second kind of background; `fg` and `bg` choose where it *resumes*.

You cannot type `bg` while a job is in the foreground, because there is no prompt.
There is no direct running-foreground to running-background transition from the keyboard.
Ctrl-Z first, then `bg`.
Ctrl-Z does not apply to a job already in the background (it hits the foreground process group).
Pause a background job with `kill -TSTP %n` (same signal family as Ctrl-Z) or `kill -STOP %n` (cannot be caught).

### Job state transitions

Three stable states: running foreground, running background, and stopped.

```
  start:  cmd      -->  running FG
          cmd &    -->  running BG

                 Ctrl-Z
     running FG ----------> stopped
          ^                  |    |
          |                  |    |
       fg |               fg |    | bg
          |                  |    |
          +----- running BG <+----+
                     |
                     | kill -STOP %n
                     | (or kill -TSTP %n)
                     v
                  stopped
```

- Ctrl-Z: running FG to stopped (pause, prompt returns).
- `bg`: stopped to running BG (continue, keep the prompt).
- `fg`: stopped or running BG to running FG (continue and take the terminal).
- `kill -STOP %n`: running BG to stopped.
- `cmd &`: enter running BG without passing through stopped.



## SSH with a remote command

`ssh` does not only open an interactive login.
You can pass a command string; the remote shell runs it and the output comes back to the local terminal, then `ssh` exits.

```shell
ssh alice@server 'ls | wc -l'
```

The pipeline runs on the server.
Quote it so the local shell does not interpret `|` (or `$`, globs) before `ssh` sends the string.
Single quotes keep the local side literal, which matches the quoting notes in `shell-gotchas.md`.

## tmux

Tilix and the Ubuntu terminal already give tabs and splits in a GUI window.
Those live in that window on that desktop.
tmux runs inside a tty (including SSH with no GUI).
A tmux *session* stays on the machine as long as the tmux server process does.
Detach (`Ctrl-B d`), close the terminal, SSH later, `tmux attach`, and the same windows and jobs are still there.

Order matters for remote work: SSH to the server, then start tmux *on the server*.
The session then belongs to that machine.
tmux first, then SSH, only wraps the connection in a local session; the remote jobs follow the SSH process unless tmux is also running on the server.

Laptop tmux and server tmux are two separate servers with two `tmux ls` lists.

One user on one host can have several sessions.
`tmux` with no arguments creates a new one.
`tmux ls` lists them.
`tmux attach` (or `tmux a`) joins the most recently used session if you omit a name.
`tmux attach -t compile` (or `tmux new -s compile`) picks or names a session from any computer that SSHs as that user.

`Ctrl-B` then a *digit* (`0`–`9`) goes to that window.
The lecture’s `N` is a placeholder for the window number.

Close a window with `Ctrl-B &` (confirm) or by exiting every pane (`exit` / Ctrl-D).
Close a pane with `Ctrl-B x`.
Detach leaves the session running.

## source, bashrc, and bash_profile

`source file` (same as `. file`) runs that file’s lines in *this* shell, as if typed here.
A child `./script.sh` cannot leave aliases, functions, or `cd` behind.
`source` can, which is why `source ~/.bashrc` and `source marco.sh` reload the current session.

`rc` in `.bashrc` is runcom (run commands): a startup script of ordinary shell lines.

A login shell (SSH, `bash -l`) reads `~/.bash_profile` if it exists (else `~/.bash_login` / `~/.profile`).
An interactive non-login shell (typical Tilix tab, tmux pane, Cursor terminal) reads `~/.bashrc`.
A usual pattern is that `.bash_profile` ends with `source ~/.bashrc` so SSH still gets aliases.

## Dotfiles

Programs take settings from plain-text files whose names start with `.` (`~/.bashrc`, `~/.vimrc`, `~/.gitconfig`, `~/.tmux.conf`, `~/.ssh/config`).
`ls` hides them by default; `ls -a` shows them.
That is how you customize the system to feel like home on any machine.

A common setup is a git repo of those files plus symlinks from `~/` into the repo, so one clone plus a small install script recreates the environment.
There is a large community of public [dotfiles](https://dotfiles.github.io/) repos on GitHub to read for ideas.