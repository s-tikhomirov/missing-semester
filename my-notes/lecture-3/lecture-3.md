# Lecture 3. Vim

Questions and clarifications from the lecture.
Slide content omitted here.

## hjkl movement

`vi` was written on the ADM-3A terminal.
That keyboard used `h` `j` `k` `l` as the cursor keys (left, down, up, right).
The bindings stuck.

On a modern QWERTY keyboard, right-hand home row has index on `j`.
`j` `k` `l` fit three fingers naturally.
`h` is one key left of `j`, so the right index covers `h` and `j`; `k` and `l` stay on middle and ring.

Personal mnemonics that helped:

- `h` and `l` — along one line (outer keys on that stretch of the row).
- `j` and `k` — between lines (down and up).

Those are learning aids.
Vim mapped four hardware directions; the outer-key horizontal idea is optional.

### Beginning vs end (keyboard order vs pairs)

Reading keys left to right as `h` `j` `k` `l` looks like alternating “toward start / toward end” because the row is left, down, up, right while `j` sits before `k` on the keyboard.

Semantic pairs that match how a file feels:

- `h` and `k` — toward the start (earlier on the line, earlier in the file).
- `j` and `l` — toward the end (later on the line, later in the file).

The layout was never “begin, begin, end, end” along the key row.
It was four geometric directions.
The `hk` / `jl` grouping is a useful mental model; `h j k l` order is key placement, especially `j` before `k`.

## Word motions and Ctrl-W

Normal mode (vi word grammar):

| Key | Motion |
|-----|--------|
| `w` | start of next word |
| `W` | start of next WORD (whitespace-separated chunk) |
| `e` | end of word |
| `E` | end of WORD |
| `b` / `B` | backward, start of word / WORD |

Lowercase vs uppercase is the usual vi pattern (punctuation splits `word`; `WORD` is coarser).

`Ctrl-W` is unrelated to “shifted `w`”:

- Insert mode — delete backward by word (Unix readline / bash `unix-word-rubout`).
- Normal mode — prefix for window commands (`Ctrl-W` then `h` / `j` / … for splits).

Two lineages in one editor: vi motions in normal mode, terminal line editing in insert mode.

## Feedback while typing a command

Vim echoes pending normal-mode keys with `showcmd` (often on by default), usually bottom-right, e.g. `d` or `d5` while waiting for a motion.
That is key echo only, without plain-English command semantics.

There is no built-in “you are in delete + pick a motion” UI.
Heritage is minimal terminal UX plus composable grammar (`d` + motion, `g`/`z` prefixes, counts, text objects).
`:help` and plugins (which-key-style) fill the gap while learning.

Cancel a partial command with `Esc` (`Ctrl-[`).
That clears operator-pending state and other unfinished prefixes.
After a command finishes, undo with `u`.

`showmode` shows `-- INSERT --` and similar on the status line.

## `$` and `^` vs regex

In normal mode, `$` moves to the end of the line; `^` to the first non-blank (`0` for the first column).

In Vim search and `:substitute` patterns, `$` and `^` are end and start of line.
Same symbols, same “where on the line” idea — ex/vi shared vocabulary for motion and matching.

Shell `$` for variables is a different context.

## `/` search vs `f` / `F`

`/` (and `?`) — search the buffer with a pattern (Vim regex rules), command line at the bottom, repeat with `n` / `N`, optional `'hlsearch'`, history.

`f` / `F` — find one literal character on the current line only (`f` forward, `F` backward).
Repeat with `;` and `,`.
`t` / `T` stop before the character (“till”).

Use `/` when the match may be far away or you only know part of the text.
Use `f` / `F` when the target character is on this line (pairs, punctuation, etc.).

## Buffer, window, tab

“Text” is the content.
A buffer is Vim’s in-memory document object: lines plus editor state (modified flag, path, buffer-local options, marks).

A window is a viewport on a buffer.
Two splits on the same file are two windows, one buffer.

A tab page is a layout of windows.

Everyday “edit this file” is fine; “buffer” is the precise term for `:ls`, `:bn`, unsaved changes, and buffers with no file yet.

## `dj` vs `dd`

`dd` — linewise delete of one line.

`dj` — operator `d` plus motion `j`: delete from the cursor through the same column one line down.
Operator motions are inclusive (both endpoints count).

With the cursor on column 1, that range is all of line 1, the newline, and the character at line 2 column 1 — often experienced as two lines removed.
From mid-line, it removes the rest of the line, the newline, and part of the next line.

For one line, use `dd`.
`d2j` deletes through the motion `2j` (two lines down), which is a larger span than `dj`.

## Swap file E325 and stuck Vim

While editing, Vim keeps `.filename.swp` for recovery and to warn about concurrent edits.

E325 means a swap already exists.
If the banner says `process ID: … (STILL RUNNING)`, another Vim session still holds the file.

Choices:

| Key | Effect |
|-----|--------|
| O | open read-only here |
| E | edit anyway (two writers) |
| R | recover from swap |
| Q / A | quit this open attempt |

Prefer finding the other session, save or quit there, then open again.

If Vim was suspended with Ctrl-Z, `ps` may show state `T` (stopped).
Plain `kill PID` sends SIGTERM (15).
For a stopped job, SIGTERM is often deferred until the job runs again, so the process and swap can linger.

`kill -9 PID` sends SIGKILL (9): the kernel stops the process immediately, even when stopped.
No graceful cleanup; unsaved work may only remain in the swap until recover or delete.

After the process is gone, open the file again.
If E325 persists with no running PID, see `:help recovery`, then remove the swap when you no longer need it.

Related lecture 2 note: SIGTERM is a request; SIGKILL is handled in the kernel and skips user cleanup handlers.
