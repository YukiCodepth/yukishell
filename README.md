# YukiShell

YukiShell is a custom Linux shell written in **C** to understand how real shells work internally.
The project is built step-by-step to explore **Linux system programming, process management, and shell architecture**.

This project demonstrates how commands are interpreted and executed using system calls like `fork()`, `execvp()`, `pipe()`, and `dup2()`.

---

# Project Overview

A **shell** is a command-line interpreter that allows users to interact with the operating system.

YukiShell aims to replicate the core features of popular shells like Bash while serving as a learning project for **operating systems and system programming**.

---

# Features

## Basic Shell

* Interactive shell prompt
* Continuous command loop
* Simple built-in commands

Example:

```
YukiShell >
```

---

## Run Linux Commands

Execute system commands directly from YukiShell.

Example:

```
YukiShell > ls
YukiShell > pwd
YukiShell > whoami
```

Implemented using:

* `fork()`
* `execvp()`
* `wait()`

---

## Command Parsing

Supports commands with arguments.

Example:

```
YukiShell > ls -l
YukiShell > mkdir test
YukiShell > rm file.txt
```

---

## Built-in Commands

Certain commands must be handled by the shell itself.

Examples:

```
YukiShell > cd ..
YukiShell > exit
```

Built-ins include:

* `cd`
* `exit`
* `help`

---

## Pipes

Allow the output of one command to be used as input for another.

Example:

```
YukiShell > ls | grep txt
```

Implemented using:

* `pipe()`
* `dup2()`

---

## Output Redirection

Redirect output to files.

Example:

```
YukiShell > ls > files.txt
```

Implemented using file descriptors and `dup2()`.

---

# Project Roadmap

| Version | Feature                                  |
| ------- | ---------------------------------------- |
| V1      | Basic shell loop                         |
| V2      | Run Linux commands                       |
| V3      | Command parsing and arguments            |
| V4      | Built-in commands (`cd`, `exit`, `help`) |
| V5      | Pipes (`ls \| grep txt`)                 |
| V6      | File redirection (`>`)                   |
| V7      | Background processes (`&`)               |
| V8      | Command history                          |
| V9      | Auto-completion                          |
| V10     | Environment variables support            |
| V11     | Modular shell architecture               |
| V12     | GUI Terminal                             |
| V13     | Plugin system                            |
| V14     | AI Terminal                              |

---

# Project Structure

```
yukishell
├── yukishell.c
├── README.md
└── .gitignore
```

---

# Compilation

Compile the program using GCC:

```
gcc yukishell.c -o yukishell
```

---

# Running YukiShell

Start the shell:

```
./yukishell
```

Example session:

```
YukiShell > pwd
/home/user/projects/yukishell

YukiShell > ls
README.md  yukishell.c

YukiShell > whoami
user

YukiShell > exit
Closing YukiShell...
```

---

# Concepts Learned

This project explores several important **Linux system programming concepts**:

* Process creation using `fork()`
* Program execution with `execvp()`
* Process synchronization using `wait()`
* Command parsing
* File descriptor manipulation
* Inter-process communication using pipes

---

# Future Improvements

Planned improvements include:

* advanced command history
* tab auto-completion
* customizable shell prompt
* scripting support
* plugin support
* graphical terminal interface
* AI-powered command assistant

---

# Author

GitHub: https://github.com/YukiCodepth

---

# License

This project is intended for **learning and educational purposes**.
