# C notes

Scratch notes from exercises and chat.
General C reference for this repo.

## Command line

`argc` counts `argv` entries, including the program name.
`argv` is `char **` or `char *argv[]`: strings only.
Parse numbers with `strtol` and pass element count separately.

## Arrays and length

`sizeof(arr)` is bytes of the whole array object.
Element count: `sizeof(arr) / sizeof(arr[0])` where `arr` has array type.

In `main`, that ratio gives `n` for the test buffer.
Inside a function taking `int *arr`, use a parameter `len` or `start`/`end`.

Array names decay to a pointer to the first element when passed to functions.

## Bounds

Half-open range `[start, end)`: `start` included, `end` excluded.
Loop: `for (i = start; i < end; i++)`.
Length: `end - start`.
`mid` splits left `[start, mid)` and right `[mid, end)`.

Inclusive `lo`/`hi` uses `i <= hi` and length `hi - lo + 1`.
Pick one convention per project.

## Storage

Local array in `main`: lifetime is the `main` frame.
`static` at file scope: lasts for the whole program, visible in this `.c` file.

`#define ARR_N (sizeof(arr) / sizeof(arr[0]))` after `arr` keeps count in sync with the initializer.

## Functions

The compiler needs a declaration or definition before each call.
Prototype at the top, or define helpers above `main`, or use a header.

`int main(void)` with `return 0;` is standard.

## Pointers and increments

`arr[i++]` uses index `i`, then adds one to `i`.
`tmp[k++] = arr[i++]` copies one element and advances both indices.

## Temp buffers

`int tmp[n];` VLA on stack when `n` is small.
`malloc(n * sizeof *tmp)` and `free` for larger or general `n`.
Merge fills `tmp`, then copies back into `arr[start..end)`.
Use separate read indices `i`, `j` and write index `k` into `tmp`.

## Debug build

`gcc -g` embeds line info for GDB.
`-Wall` enables common warnings.

`#ifdef DEBUG` … `#endif` wraps verbose `printf`.
Enable with `gcc -DDEBUG` or with `#define DEBUG` in the file (choose one per build).

`assert(condition)` from `<assert.h>`; `NDEBUG` disables asserts.

Custom messages: `fprintf(stderr, "i=%d\n", i);` then `abort();`.

## GDB (minimal)

`gcc -g -o prog prog.c`
`gdb ./prog`
`break merge`
`run`
`next`, `step`, `print i`, `printf "i=%d j=%d\n", i, j`

## Preprocessor

`-DDEBUG` defines macro `DEBUG` on the compiler command line.
Most flags (`-g`, `-Wall`, `-DDEBUG`) can appear in any order.
`-o` must be followed immediately by the output filename.
