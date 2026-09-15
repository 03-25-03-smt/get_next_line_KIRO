*This project has been created as part of the 42 curriculum by vladyslb.*

# get_next_line

## Description

`get_next_line` is the second project of the 42 curriculum. The goal is to write a
function that returns **one single line** read from a file descriptor, and that can be
called repeatedly to walk through the whole input, one line at a time.

```c
char	*get_next_line(int fd);
```

| | |
| --- | --- |
| **Parameter** | `fd` — the file descriptor to read from |
| **Returns** | the line that was read, including its terminating `\n` |
| **Returns** | `NULL` when there is nothing left to read, or when an error occurs |
| **Allowed functions** | `read`, `malloc`, `free` |

The interesting part of the problem is that `read()` knows nothing about lines. It
hands back a fixed-size block of bytes that may contain several newlines, half a line,
or none at all. The function therefore has to remember, between two calls, whatever it
has read but not yet returned. That memory is kept in a **static variable**, which is
the real lesson of this project.

The returned line always ends with `\n`, except for the last line of a file that does
not end with a newline. The caller owns the returned string and must `free()` it.

### Features

- Works on regular files, on standard input, and on pipes and FIFOs.
- Works for any `BUFFER_SIZE`, from `1` to values far larger than the file itself, and
  compiles with or without the `-D BUFFER_SIZE` flag.
- Reads lazily: only as many bytes as are needed to complete the current line, so it
  can be used on a terminal or a pipe.
- Returns `NULL` for a file descriptor that cannot be read from — negative, unopened,
  closed, opened for writing only, or pointing at a directory — instead of handing back
  leftover data from an earlier descriptor.
- Frees the stash on every exit path, including when `read()` fails.
- The bonus version handles several file descriptors at the same time, using a single
  static variable.

## Files

All files are at the root of the repository, as required by the subject.

| File | Contents |
| --- | --- |
| `get_next_line.c` | `get_next_line()` and its two static helpers |
| `get_next_line_utils.c` | the helper functions (length, search, copy, join) |
| `get_next_line.h` | the `get_next_line()` prototype, the helper prototypes and the `BUFFER_SIZE` fallback |
| `get_next_line_bonus.c` | bonus version, multiple file descriptors |
| `get_next_line_utils_bonus.c` | helpers for the bonus version |
| `get_next_line_bonus.h` | header for the bonus version |

There is no `Makefile`: the subject asks only for the three source files at the root of
the repository and specifies a direct `cc` invocation, so a build file is not part of
the deliverable.

## Instructions

### Compilation

The project has no dependency other than the C standard library. Compile your own
`main.c` together with the two source files:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line.c get_next_line_utils.c -o gnl
```

`BUFFER_SIZE` is the number of bytes handed to each `read()` call. Any strictly
positive value works. The project also compiles **without** the flag, in which case
the header falls back to `42`:

```bash
cc -Wall -Wextra -Werror main.c get_next_line.c get_next_line_utils.c -o gnl
```

### Bonus

The bonus version has the same prototype, so only the file names change:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 main.c get_next_line_bonus.c get_next_line_utils_bonus.c -o gnl_bonus
```

### Usage

```c
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "get_next_line.h"

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("file.txt", O_RDONLY);
	if (fd < 0)
		return (1);
	line = get_next_line(fd);
	while (line)
	{
		printf("%s", line);
		free(line);            /* the caller owns the line */
		line = get_next_line(fd);
	}
	close(fd);
	return (0);
}
```

Reading from the standard input works the same way, with `fd` set to `0`:

```bash
printf 'alpha\nbeta\n' | ./gnl
```

## Algorithm

### The problem `read()` leaves behind

`read(fd, buf, BUFFER_SIZE)` returns an arbitrary block of bytes. A single call may
return several lines at once, or a fragment of a line, and the newline that ends the
line the caller asked for may arrive long before or long after the end of the block.
Two things therefore have to be solved: **accumulating** bytes until a newline is
found, and **remembering** the surplus bytes for the next call.

### The chosen approach: one accumulating stash in a static variable

The implementation keeps a single `static char *buffer` — the *stash* — holding
everything that has been read from the descriptor but not yet handed back to the
caller. A call to `get_next_line()` is then three steps.

**1. Validate the descriptor.** `fd` must not be negative and `BUFFER_SIZE` must be
strictly positive. On top of that, `read(fd, &test, 0)` is used as a probe: a request
of zero bytes performs no I/O and consumes no input, but it still fails with `-1` on a
descriptor that is closed, never opened, opened for writing only, or pointing at a
directory. If the probe fails, the stash is freed and the function returns `NULL`.

This step matters more than it looks. Without it, the newline check in step 2 would
short-circuit the read whenever the stash already held a complete line, and a call on a
broken descriptor would hand back a line belonging to a *previously read file*.

**2. Fill the stash** (`get_current_buffer`). While the stash contains no `\n` and
`read()` still returns data, read `BUFFER_SIZE` bytes and append them to the stash. The
loop stops on the *first* newline seen, so nothing is read ahead of what the current
line needs. If `read()` returns `-1`, both the temporary block and the stash are freed
and the function returns `NULL`.

**3. Split the stash** (`get_line`). Copy everything up to and including the first `\n`
into a new string — the line to return — and copy the remainder into a fresh stash that
replaces the old one. If the remainder is empty, the stash is freed and set back to
`NULL`, so the next call starts clean.

The helpers in `get_next_line_utils.c` are deliberately minimal:

| Helper | Role |
| --- | --- |
| `strlen_at` | measures a string up to either `\0` or a given byte, which serves both to locate a newline and to measure the whole stash |
| `find_chr` | tells whether the stash already holds a complete line |
| `ft_memcpy` | raw block copy, used by the join |
| `cpy_buffer` | extracts a range of the stash into a new allocation |
| `merge_previous_and_current` | concatenates the stash with a freshly read block and frees the old stash, including when its own allocation fails |

### Why this design

- **A static variable is the only place to keep state.** The prototype takes nothing
  but an `fd` and returns a `char *`, so the surplus bytes cannot be given back to the
  caller, and global variables are forbidden by the subject. A function-scoped
  `static` is the narrowest scope that survives between calls.

- **Reading lazily is a requirement, not an optimisation.** Stopping at the first
  newline is what makes the function usable on a terminal or a pipe: if it drained the
  input to `EOF` before returning, reading interactively would never produce a line
  until the stream closed. Verified in testing: after one call on a 100 000-byte file
  the descriptor's offset is exactly `BUFFER_SIZE`, not the file size.

- **A zero-byte `read()` as the validity probe.** `read` is one of the three functions
  the subject allows, so the check costs nothing in terms of permitted tools. POSIX
  guarantees that a request of zero bytes performs no transfer, which means the probe
  cannot swallow a byte of input, yet the descriptor is still checked and `EBADF` or
  `EISDIR` still surfaces. `fstat()` or `fcntl()` would have answered the same question
  but are not allowed here.

- **Storing the surplus as a plain string, not a list of blocks.** A linked list of
  chunks would avoid re-copying the stash on every `read()`, but it would need a
  structure, node management and a flattening pass, none of which fit in 25-line
  functions. Since the stash only ever holds the bytes of the *current* line plus at
  most one extra block, the amount copied stays proportional to the line length. The
  simpler representation is the better trade-off here.

- **Two separate helpers instead of one big function.** `get_current_buffer` only
  grows the stash, `get_line` only cuts it. Keeping "get more bytes" apart from "hand
  back one line" is what allows each of them to stay well inside the Norm's 25-line
  limit, and it makes the newline logic testable in isolation.

- **Freeing on every exit path.** A `read()` error, a failed allocation inside the
  join, or a failed split all release the stash before returning `NULL`. Without that,
  an error in the middle of a file would abandon everything accumulated so far.

- **Independence from `BUFFER_SIZE`.** No part of the logic assumes a block contains a
  whole line, a single line, or any newline at all. `BUFFER_SIZE = 1` reads one byte at
  a time and simply loops more often; `BUFFER_SIZE = 10000000` reads the file in a
  single call and the stash is split across the following calls without touching the
  descriptor again. Both produce byte-for-byte identical output.

### Bonus: several descriptors at once

The bonus version replaces the single stash with `static char *buffer[MAX_FILES]`,
indexed by file descriptor — still **one** static variable, as the bonus requires.
Each descriptor keeps its own reading state, so calls can be interleaved across fds 3,
4 and 5 in any order without mixing lines, and any `fd` outside the array's range
returns `NULL`. `MAX_FILES` is defined as `OPEN_MAX` from `<limits.h>`, the system's
limit on simultaneously open descriptors.

An array indexed by `fd` was chosen over a linked list of per-descriptor states because
lookup is O(1) and needs no allocation of its own: nothing can fail while locating the
stash for a descriptor, which keeps the error paths as simple as in the mandatory part.

### Complexity

For a line of length `L`, the function performs `⌈L / BUFFER_SIZE⌉` calls to `read()`
and copies `O(L)` bytes per call in `merge_previous_and_current`, for `O(L² /
BUFFER_SIZE)` byte copies in the worst case of a very long line read with a very small
buffer. Memory use stays `O(L + BUFFER_SIZE)`: only the current line and one block are
ever held at once.

## Testing

The submitted files contain no test code. Testing was done with a separate harness that
compares `get_next_line()` byte-for-byte against POSIX `getline()` on the same file,
which gives an unambiguous reference for the expected output.

Cases covered:

- empty file, a file holding only `\n`, consecutive empty lines
- a file ending with `\n` and a file not ending with `\n`
- a single 100 000-character line, and 60 randomly generated files
- `BUFFER_SIZE` set to 1, 2, 3, 5, 7, 42, 100, 1024, 9999 and 10000000, plus
  compilation with no `-D BUFFER_SIZE` at all
- standard input, pipes and FIFOs
- negative, unopened, closed, write-only and directory file descriptors
- repeated calls after end of file
- interleaved reads on three descriptors at once, for the bonus
- allocation failures, injected by intercepting `malloc` at link time, to check that
  the error paths do not abandon the stash

Results: every case matches `getline()` at every buffer size, with no leaks and no
undefined behaviour reported by `-fsanitize=address,undefined`. The sources pass
`norminette` with no errors, and compile without warnings under
`-Wall -Wextra -Werror`, including with `-std=c99 -pedantic`.

Community testers for this project:

- [francinette](https://github.com/xicodomingues/francinette)
- [gnlTester](https://github.com/Tripouille/gnlTester)
- [get_next_line_tester](https://github.com/Mazoise/42TESTERS-GET_NEXT_LINE)

## Resources

Reference documentation and reading used while working on the project:

- `man 2 read`, `man 3 malloc`, `man 2 open` — the behaviour of `read()` at end of
  file and on error is the core of the project
- [POSIX specification of `read()`](https://pubs.opengroup.org/onlinepubs/9699919799/functions/read.html)
  — in particular the guarantee that a request of zero bytes performs no transfer but
  may still report an error, which is what the descriptor probe relies on
- Brian W. Kernighan, Dennis M. Ritchie, *The C Programming Language*, chapter 5
  (pointers and arrays) and chapter 7 (input and output)
- [42 Norm (v4)](https://cdn.intra.42.fr/pdf/pdf/960/norm.en.pdf) — the formatting and
  25-lines-per-function rules the code has to satisfy
- POSIX `getline()` documentation, used as the behavioural reference the tests compare
  against

### Use of AI

The function itself — the stash kept in a static variable, the split between
`get_current_buffer` and `get_line`, and the helper functions — was designed and
written by hand, without AI. AI assistance (Kiro CLI, Claude) was used afterwards, in
this order:

- **Testing.** Writing the `getline()`-comparison harness, the random file generator,
  the sanitizer and `BUFFER_SIZE` sweep, and the link-time `malloc` interception
  described in *Testing*. None of that code is part of the submission.
- **Reporting defects, not fixing them.** That testing surfaced two problems: the stash
  was not freed when `read()` or an allocation failed, and a call on an unreadable
  descriptor could return a leftover line from a previously read file. Both were
  reported with the failing test cases; the corrections in the source — the `free()`
  calls on the error paths and the zero-byte `read()` probe — were written by hand.
- **Norm formatting.** Reformatting the finished sources to satisfy the 42 Norm:
  converting space indentation to tabs, moving comments out of function bodies,
  shortening lines over 80 columns and adding the 42 headers. No logic was changed by
  this step, which was verified by comparing the preprocessed, comment-stripped code
  against the previous version: all six files came out byte-for-byte identical.
- **Documentation.** Drafting this `README.md`.

AI was not used to obtain the algorithm itself, which is the part of the project that
carries the learning.
