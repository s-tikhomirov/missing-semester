# Lecture 4. Debugging and Profiling

Clarifications from chat while watching the lecture.

## Debuggers and symbols

GDB and LLDB both debug programs: breakpoints, stepping, inspecting state.
GDB pairs with GCC on Linux; LLDB pairs with Clang and Xcode on macOS.

The CPU runs machine code.
`-g` is a compiler flag for GCC and Clang (e.g. `gcc -g -o prog prog.c`).
It maps instruction addresses in the binary to source lines, variables, and stack frames.
`-g` adds metadata for tools like GDB.
Sanitizers and the debugger use other flags or commands.
Pick optimization separately (`-O0`, `-O2`, …).
Without `-g`, the debugger shows instruction addresses and assembly.
Source lines and variable names are missing or unreliable.

## Record / replay vs normal GDB

Default GDB moves forward (`next`, `step`, `continue`).

Record/replay (e.g. rr) saves one run.
You replay it and can step backward through that history.

GDB also has optional reverse stepping (`record`).
That mode is separate from the default.

## rr on this machine (corruption exercise)

Ubuntu 24.04 `apt install rr` ships 5.7.0.
13th Gen i7-13700H (CPU id `0xb06a0`) needs rr 5.9.0 for Raptor Lake support in `PerfCounters_x86.h`.
Upstream `.deb` from GitHub releases works without building from source.
`sudo apt remove rr` removes a dpkg-installed package.

`kernel.perf_event_paranoid` defaults to 4 on this system.
rr 5.9.0 wants `<= 3` (5.7.0 asked for `<= 1` or `-n`).
`sudo sysctl kernel.perf_event_paranoid=1` fixes the sysctl check.

After that, recording still failed: `Got 0 branch events, expected at least 500`.
`perf stat -e r5111c4 true` showed `cpu_core/...` as not counted (0%).
Hybrid Intel (P-cores vs E-cores) and kernel perf support on the XPS 15 9530 block rr here.
Corruption exercise fallback: GDB watchpoint on `students[1].id` and backtrace in `curve_scores`.

## Sanitizers

Linters analyze source statically.
Sanitizers instrument the binary and check memory and behavior at runtime.

Many memory bugs run on with wrong data.
Some crash far from the bug.
Sanitizers stop at the illegal access and print a report (clearer with `-g`).

The compiler inserts checks; a runtime library enforces them.
GCC and Clang both support `-fsanitize=...`.

Release builds skip sanitizers by default.
AddressSanitizer often costs ~1.5–3× CPU and ~2–3× RAM.

Shadow memory is a small metadata map beside real memory.
Each entry marks a region as valid or poisoned.

ASan rarely flags correct code on simple sanitized C programs.
It can miss overflows that land inside another live object past the red zone.

AddressSanitizer report vocabulary (e.g. `uaf` exercise):

Shadow byte: one metadata byte in the shadow map for a chunk of app memory.
Red zone: poisoned padding before and after an allocation; touches there report overflow.
Addressable: byte ASan treats as valid for access now (live heap, stack, global).
Partially addressable: one access spans valid bytes and poisoned bytes (e.g. int read across the end of an object).

Build: `gcc -g -fsanitize=address -o uaf uaf.c`.

## GDB quick use (exercises)

`gcc -g -Wall -o sort sort.c`
`gdb ./sort`
`break merge` or `break sort.c:LINE`
`run`
`next`, `step`, `continue`
`print i` or `printf "i=%d j=%d\n", i, j`
`info locals`
`watch students[1].id` for corruption when rr is unavailable

## User space, kernel, eBPF

User space holds normal processes.
They run unprivileged.
They use syscalls to ask the OS for files, memory, and network.

Kernel space holds the OS core.
It runs privileged.
Your `main()` loop runs in user space.

eBPF lives in the kernel.
User-space tools (bpftrace, bcc) load small eBPF programs onto kernel hooks.

Each syscall still runs kernel code for your process.
eBPF adds observation on that path.
A user-space tool reads maps and buffers afterward.

## strace vs bpftrace

Both help you see syscalls.
They differ in mechanism and scope.


|                 | strace                                                             | bpftrace                       |
| --------------- | ------------------------------------------------------------------ | ------------------------------ |
| Mechanism       | ptrace                                                             | eBPF on tracepoints            |
| Syscall view    | Stops at entry and exit; reads registers and args; prints; resumes | Runs hooks on the syscall path |
| Overhead        | High                                                               | Lower                          |
| Scope           | One process (straightforward attach)                               | Whole system with filters      |
| Beyond syscalls | Syscall-focused                                                    | Kernel and user probes         |
| Privilege       | Often unprivileged for own processes                               | Usually root                   |
| Aggregation     | Line-by-line log                                                   | Counts, histograms             |


List probe names with `bpftrace -l 'tracepoint:syscalls:*'`.
Example: `tracepoint:syscalls:sys_enter_*`.

strace prints its trace on stderr so the traced program keeps stdout for pipes and `> file`.
`strace ./app | grep open` greps only the app’s stdout.
`strace ./app 2>&1 | grep open` greps the syscall log.
`strace -e openat,read ./app 2>&1` filters syscalls inside strace.

## Exercises (this repo)

`exercise-1-sort`: merge sort in C, half-open `[start, end)`, temp buffer inside `merge`, GDB in `merge`.
`exercise-2-corruption`: buffer overflow in `curve_scores`; rr blocked on this laptop (see above); GDB watchpoint.
`exercise-3-uaf`: use-after-free; AddressSanitizer report and line number with `-g`.
strace / bpftrace exercises: remember `2>&1` when piping strace.

## gcc flags (profiling builds)

`-g`: compiler flag to emit debug metadata (DWARF) for GDB and sanitizer line numbers.
`-o name`: name the output executable; default name without `-o` is `a.out`.
`-lm`: linker flag `-l` plus library short name `m` → libm (`math.h`).
`-lname` links libname.so for any installed library; `-lpthread`, `-lcurl`, etc.

## perf and flame graphs

`perf record` may warn about `kernel.kptr_restrict`: kernel stack frames stay unresolved without symbols; user-space profiling still works.

Flame graph scripts: clone [FlameGraph](https://github.com/brendangregg/FlameGraph) (Brendan Gregg).
Pipeline: `perf record -g ./prog`, then `perf script | stackcollapse-perf.pl | flamegraph.pl > flamegraph.svg`.
Needs Perl; put the repo on `PATH` or call scripts by path.
Alternatives: `perf report`, Speedscope in the browser.

## hyperfine (grep vs rg)

Pick a fixed directory and pattern; hyperfine only times the commands you pass.
Use the same workload for both tools, e.g. `hyperfine --warmup 3 'rg PAT dir' 'grep -r PAT dir'`.
Align behavior: `rg` honors `.gitignore` by default; `grep -r` searches more files unless you scope or use `rg --no-ignore`.

## taskset and stress

`stress -c 3` starts three CPU load workers.
`taskset --cpu-list 0,2` pins them to logical CPUs 0 and 2 only (two CPUs).
At most two workers run on cores at once; the third shares time slices on 0 and 2 with the others.
For three busy cores, allow three CPUs in `taskset` or drop `taskset`.