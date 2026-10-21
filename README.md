# TechShell

A basic Unix shell written in C. It displays a prompt, runs commands, and supports the built-in `cd` command and input/output redirection.

**Authors:** Connor McKennan, Jacob Wolfe, Antonio Hernandez
**Date:** February 2026
**Course:** [course name and number, e.g., CSC 3XX Operating Systems]

## Features

- Shows the current working directory in the prompt, and updates it after `cd`
- Runs any program found in your PATH, with arguments (`ls -l`, `gcc main.c`)
- Built-in `cd <directory>` command
- Input redirection with `<` (`sort < input.txt`)
- Output redirection with `>` (`ls > files.txt`)
- Clean exit with Ctrl+D
- Error messages for failed commands, missing directories, and missing filenames

## Build and Run

Requires a Unix-like system (Linux or macOS) and GCC.

```bash
gcc -Wall -o techshell techshell.c
./techshell
```

## Example Usage

```
/home/user$ ls -l
/home/user$ cd Documents
/home/user/Documents$ ls > files.txt
/home/user/Documents$ sort < files.txt
/home/user/Documents$ notacommand
Error 2 (No such file or directory)
```

## How It Works

1. **`CommandPrompt`** prints the current directory, reads a line with `fgets`, and strips the newline.
2. **`ParseCommandLine`** splits the line into tokens and fills a `ShellCommand` struct with the program name and arguments (`argv`), the argument count, and optional input and output filenames.
3. **`main`** handles `cd` directly, since a child process can't change the shell's own directory. Every other command is passed to the next step.
4. **`ExecuteCommand`** uses `fork` to create a child process. The child sets up any redirection with `freopen`, then calls `execvp` to run the program. The parent waits with `waitpid`.
5. **`FreeCommand`** releases all memory allocated during parsing, so the shell doesn't leak memory.

## My Contribution

**Jacob Wolfe:** Wrote `CommandPrompt` and `ParseCommandLine`: prompt display, safe input reading, tokenizing, redirection handling, and building the null-terminated `argv` array that `execvp` requires. [Add anything else you did, such as testing or debugging.]

**Connor McKennan:** [their part]

**Antonio Hernandez:** [their part]

## Design Notes

- Input is read with `fgets` and a fixed buffer size to avoid buffer overflows.
- Every `realloc` and `strdup` call is checked for failure.
- `realloc` uses a temporary pointer so the original array isn't lost if allocation fails.
- Parsed strings are copied with `strdup`, so they stay valid after the input buffer is reused.

## Known Limitations

- No quote handling (`echo "hello world"` is split into two arguments)
- `<` and `>` must have spaces around them
- No pipes (`|`), background jobs (`&`), or environment variable expansion
- Input lines are limited to 1,023 characters
- If a redirection operator has no filename, an error is printed but the command still runs

## Possible Improvements

- Quote and escape handling in the parser
- Pipes and background processes
- Command history
- Support for appending output with `>>`
