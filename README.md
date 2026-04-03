# mysh — A Custom Unix Shell in hw2 folder

A Unix shell built from scratch in C for CS 3502: Operating Systems. Implements core shell functionality including process creation, file descriptor manipulation, and signal handling.

## Build & Run

```bash
gcc -o mysh mysh.c
./mysh
```

## Features

### Core Loop
Prints a `mysh> ` prompt, reads input, executes commands, and repeats. Handles `Ctrl-D` (EOF) for clean exit.

### Command Execution
Forks a child process and uses `execvp` to run any program available on the system.

```
mysh> ls -l
mysh> gcc -o program program.c
```

### Built-in Commands
- `cd [dir]` — change directory (defaults to `$HOME` if no argument given)
- `exit` — exit the shell

### Output Redirection (`>`)
Redirects stdout to a file, creating or overwriting it.

```
mysh> ls > out.txt
mysh> echo hello > greeting.txt
```

### Input Redirection (`<`)
Redirects stdin from a file.

```
mysh> sort < fruits.txt
mysh> wc -l < fruits.txt
```

### Pipes (`|`)
Connects the stdout of one command to the stdin of another.

```
mysh> ls | grep mysh
mysh> echo hello world | wc -w
mysh> cat mysh.c | head -5
```

### Background Processes (`&`)
Runs a command in the background without blocking the shell. Zombie processes are automatically reaped via a `SIGCHLD` handler.

```
mysh> sleep 5 &
mysh> # prompt returns immediately
```

## Implementation Notes

- Tokenization via `strtok` into a NULL-terminated `args` array
- Redirection and pipe tokens are scanned and stripped from `args` before `execvp`
- Pipe implemented with `pipe()`, two `fork()` calls, and `dup2()` for fd wiring
- Background processes skip `waitpid`; a `SIGCHLD` handler with `WNOHANG` reaps finished children
