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


# Thread Pool — In hw3 folder

A POSIX thread pool implementation in C built for the OS course at KSU. The pool reads tasks from stdin, distributes them across a fixed number of worker threads through a shared queue, and prints results to stdout. Includes optional dynamic scaling as a bonus level.

## How It Works

The main thread acts as the producer — it reads lines from stdin and pushes them onto a shared linked list queue. Worker threads act as consumers, pulling tasks off the queue and processing them concurrently. The "work" each task does is computing a djb2 hash of the input string after a simulated 100ms delay, which lets the grader verify true concurrency by measuring wall-clock time.

The shared queue is the core of the design. Multiple threads accessing it simultaneously makes it a classic critical section, protected here with a mutex and a condition variable.

## Architecture

**Producer-Consumer Queue** — a linked list queue shared between the main thread and all workers. The main thread pushes tasks onto the tail, workers pop from the head.

**Mutex Protection** — every read and write of the queue is wrapped in a lock/unlock pair so no two threads corrupt the linked list simultaneously. The mutex is released before processing each task so workers can run concurrently instead of serializing on the lock.

**Condition Variable** — instead of busy-waiting when the queue is empty, workers sleep on a condition variable and wake only when the main thread signals that a new task has been pushed. This keeps CPU usage near zero during idle periods.

**Clean Shutdown** — when stdin closes, the main thread sets a done flag and broadcasts on the condition variable to wake all sleeping workers. Workers drain any remaining tasks from the queue before exiting. The main thread joins all workers before returning.

**Dynamic Scaling (Bonus)** — when queue depth exceeds twice the base worker count, new threads are spawned to handle the burst. When the burst subsides and extra workers find the queue empty, they exit and the pool shrinks back to its baseline size.

## Build and Run

```bash
gcc -o threadpool threadpool.c -pthread
echo -e "hello\nworld\nfoo" | ./threadpool 4
```

The single argument is the number of worker threads. Tasks are read from stdin, one per line.

## Concepts Demonstrated

- POSIX threads with `pthread_create` and `pthread_join`
- Mutex synchronization with `pthread_mutex_t`
- Condition variables with `pthread_cond_wait`, `pthread_cond_signal`, and `pthread_cond_broadcast`
- Producer-consumer pattern with a bounded shared queue
- Spurious wakeup handling with while-loop condition checks
- Clean resource management with mutex and condition variable destruction
- Dynamic thread spawning and pool scaling under load
