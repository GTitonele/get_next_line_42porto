*This project has been created as part of the 42 curriculum by gbaccell.*

<div align="center">

# 📖 get_next_line

**Reading a line from a file descriptor is way too tedious.**

![Language](https://img.shields.io/badge/language-C-blue?style=flat-square)
![Norminette](https://img.shields.io/badge/norminette-OK-success?style=flat-square)
![Bonus](https://img.shields.io/badge/bonus-multi--fd-purple?style=flat-square)
![School](https://img.shields.io/badge/42-Porto-black?style=flat-square)

</div>

---

## 📌 Description

`get_next_line` is a C function that returns **one line at a time** from a file descriptor:

```c
char *get_next_line(int fd);
```

Each call returns the next line of the file (including the trailing `\n`, if there is one).
When there is nothing left to read, or an error happens, it returns `NULL`.

The main goal of the project is to learn about **static variables** in C: the function has to
remember what it has already read *between calls*, without global variables, `lseek()` or libft.
It also teaches buffered I/O with `read()`, careful dynamic memory management, and edge cases
such as files without a final newline, empty files, standard input and very small or very large
buffer sizes.

### ✨ Features

| | Mandatory | Bonus |
|---|:---:|:---:|
| Reads one line per call | ✅ | ✅ |
| Works with files **and** standard input | ✅ | ✅ |
| Any `BUFFER_SIZE` (1, 42, 9999, 10 000 000…) | ✅ | ✅ |
| Compiles with **or** without `-D BUFFER_SIZE` (default: `42`) | ✅ | ✅ |
| No memory leaks | ✅ | ✅ |
| Only **one** static variable | ✅ | ✅ |
| Several file descriptors at the same time | ❌ | ✅ |

### 📂 Files

```
.
├── get_next_line.h              # prototypes + default BUFFER_SIZE
├── get_next_line.c              # get_next_line() and its static helpers
├── get_next_line_utils.c        # ft_strlen, ft_strchr, ft_strjoin, ft_substr
├── get_next_line_bonus.h        # bonus header (adds MAX_FD)
├── get_next_line_bonus.c        # multi-fd version
└── get_next_line_utils_bonus.c  # same helpers, bonus build
```

---

## 🛠️ Instructions

There is no Makefile (the subject does not require one): the function is meant to be compiled
together with your own program.

### Compile

```bash
# Mandatory part
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c main.c

# Bonus part
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line_bonus.c get_next_line_utils_bonus.c main.c
```

`-D BUFFER_SIZE=n` is optional; if omitted, the header defaults to `42`.

### Changing the line separator (optional)

The character that ends a line is the `SEP` macro, defined once in the headers:

```c
# ifndef SEP
#  define SEP '\n'
# endif
```

By default it is `'\n'`, which is the behaviour required by the subject. To split on another
character, change that line or define it at compile time:

```bash
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 -D SEP="';'" get_next_line.c get_next_line_utils.c main.c
# "abc;def;ghi" → "abc;", "def;", "ghi", NULL
```

### Usage example

```c
#include <fcntl.h>
#include <stdio.h>
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
		free(line);               /* the caller owns each returned line */
		line = get_next_line(fd);
	}
	close(fd);
	return (0);
}
```

```bash
./a.out                    # read file.txt
```

To read from standard input, simply pass `0` as the file descriptor.

### Check for leaks

```bash
cc -g -D BUFFER_SIZE=1 get_next_line.c get_next_line_utils.c main.c
valgrind --leak-check=full ./a.out
```

---

## 🧠 Algorithm

### The idea: a *stash* that survives between calls

`read()` returns blocks of `BUFFER_SIZE` bytes, which almost never match line boundaries.
A single read can return half a line, or several lines at once. So the function keeps the
leftover bytes in a **static string (the *stash*)** that persists from one call to the next.

```
 file:   "Hello\nWorld\n42"            BUFFER_SIZE = 4

 call 1  read "Hell" → stash "Hell"         no '\n' → read again
         read "o\nWo" → stash "Hello\nWo"   '\n' found → stop reading
         return "Hello\n"                   stash keeps "Wo"

 call 2  read "rld\n" → stash "World\n"    '\n' found → stop
         return "World\n"                   nothing left → stash = NULL

 call 3  read "42"   → stash "42"
         read → 0 (EOF)                     stop reading
         return "42"                        stash freed → NULL

 call 4  return NULL
```

### Step by step

Each call to `get_next_line()` does three things:

1. **`ft_read_stash`**: reads `BUFFER_SIZE` bytes at a time and appends them to the stash,
   **stopping as soon as the stash contains a `\n`** or `read()` returns `0` (EOF).
   If `read()` fails (`-1`), the stash is freed and `NULL` is returned.
2. **`ft_get_line`**: copies everything from the start of the stash up to and including the
   first `\n` (or up to the end, on the last line). This is the returned line.
3. **`ft_new_stash`**: keeps only what comes after that `\n` for the next call and frees the
   old stash. If nothing is left, the stash becomes `NULL`.

### Why this approach?

- **Reads as little as possible.** The loop stops at the first newline, as the subject asks.
  The function never loads the whole file and works on infinite streams (`stdin`, pipes).
- **Only one static variable.** All the state lives in the stash; the read buffer is a local
  `malloc` freed before returning, so a huge `BUFFER_SIZE` (e.g. 10 000 000) does not
  overflow the stack.
- **Independent of `BUFFER_SIZE`.** With `1` it reads byte by byte; with a huge value it
  usually needs a single `read()`. The logic is the same either way.
- **Simple memory ownership.** `ft_strjoin` frees its first argument, so the stash is never
  leaked when it grows. All error cases in `ft_read_stash` (failed `malloc`, `read` returning
  `-1`, failed `ft_strjoin`) leave the loop with `break` and are handled in one single place
  at the end of the function, which frees what is left and returns `NULL`.
  `get_next_line` also checks whether a leftover was expected (`has_rest`): if saving it
  fails, the line is freed and `NULL` is returned instead of silently losing data.

### Known limitations

- **Quadratic copying on very long lines.** `ft_strjoin` copies the whole stash every time a
  new buffer is appended, so a line of *n* characters read with a small `BUFFER_SIZE` costs
  about n² / (2 × `BUFFER_SIZE`) copied bytes. It is fast in normal use, but the
  `francinette --strict` malloc-failure test on a 20 000-character line with `BUFFER_SIZE=10`
  times out for this reason (the results themselves are correct and leak-free).

### Bonus: several file descriptors

The bonus keeps a single static variable that is now an **array of stashes indexed by fd**:

```c
static char	*stash[MAX_FD];   /* MAX_FD = 1024 by default */
```

Each file descriptor has its own stash, so you can alternate `fd 3 → fd 4 → fd 5 → fd 3…`
without one file's leftovers mixing with another's. Invalid descriptors
(`fd < 0` or `fd >= MAX_FD`) return `NULL`.

---

## 📚 Resources

- [`man 2 read`](https://man7.org/linux/man-pages/man2/read.2.html): how `read()` behaves (partial reads, `0` on EOF, `-1` on error)
- [`man 3 malloc`](https://man7.org/linux/man-pages/man3/malloc.3.html) and `free`
- [Static variables in C (GeeksforGeeks)](https://www.geeksforgeeks.org/static-variables-in-c/)
- [File descriptors (Wikipedia)](https://en.wikipedia.org/wiki/File_descriptor)
- [Valgrind quick start](https://valgrind.org/docs/manual/quick-start.html): finding memory leaks
- The 42 Norm (`norminette`)

### 🤖 Use of AI

AI (Claude) was used throughout the project as a tutor:

- **Concepts:** explanations of static variables, file descriptors, `read()`, stack vs heap
  memory and buffer overflows, with small experiments to run (e.g. reading a file 5 bytes
  at a time, a step-by-step simulation of the stash).
- **Step-by-step guidance:** I wrote the header and each helper function myself
  (`ft_strlen`, `ft_strchr`, `ft_strjoin`, `ft_substr`, `ft_get_line`, `ft_new_stash`,
  `get_next_line`); the AI reviewed every version, pointed out bugs (missing `NULL` checks,
  leaks, uninitialised variables, out-of-bounds reads, Norm issues) and asked guiding
  questions instead of giving the answer directly.
- **Debugging:** the AI helped analyse a `francinette --strict` failure and found that a failed
  `malloc` inside `ft_strjoin` made the function return a wrong line instead of `NULL`.
  The restructured error handling in `ft_read_stash` and the bonus version of
  `get_next_line` (array of stashes indexed by fd, `has_rest` check) were proposed by the AI with
  explanations, then reviewed, typed and tested by me.
- **Testing and documentation:** help designing test cases (different `BUFFER_SIZE` values,
  empty files, files without a final `\n`, standard input, invalid fd, several fds,
  `valgrind`) and writing this README.
