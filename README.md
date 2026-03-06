# YUKISHELL

> An AI-Powered, Embedded-First Linux Terminal Environment

YukiShell is a next-generation custom operating system shell built entirely in C. It is designed from the ground up to serve as the ultimate unified workspace for Embedded Systems engineers, IoT developers, and Edge AI workflows. 

Currently, developers must constantly switch between standard shells (Bash/Zsh), serial monitors (Minicom), hardware flashers, and AI interfaces. YukiShell merges the core of a POSIX-compliant Linux shell with native hardware debugging and local AI integration, acting as the foundation for a future custom operating system.

---

## CURRENT STATUS: PHASE 1 (CORE ENGINE) - COMPLETED

The foundational C architecture is fully modular, memory-safe, and capable of handling standard OS-level process management. 

### Core Capabilities
* **Custom Execution Engine:** Handles `fork()`, `execvp()`, and process state management directly with the Linux kernel.
* **Advanced Data Routing:** Native support for process pipelines (`|`) and output redirection (`>`).
* **Job Control:** Background process execution (`&`) with automated Zombie Process cleanup via `SIGCHLD` signal handling.
* **Modern UX:** Integrated with GNU `readline` for Up/Down arrow command history and Tab-autocomplete.
* **Modular Architecture:** Professionally decoupled into isolated C modules (`parser`, `executor`, `builtins`) managed by a custom `Makefile`.

---

## BUILD & INSTALLATION

### Prerequisites
You will need the GNU Compiler Collection (`gcc`), `make`, and the `readline` library installed on your system.

```bash
sudo apt update
sudo apt install build-essential libreadline-dev
Compilation
Clone the repository and build the executable using the provided build system:

Bash
git clone [https://github.com/yourusername/yukishell.git](https://github.com/yourusername/yukishell.git)
cd yukishell
make clean
make
./yukishell
MASTER ROADMAP
Phase 1: Core Shell (Completed)
Process management, pipes, redirection, background jobs, and modular architecture.

Phase 2: Beautiful Terminal (In Progress)
ANSI color integration, dynamic system prompts (Git branch, directory, hostname), and syntax highlighting.

Phase 3: AI Terminal Integration
Built-in local LLM hooks for command prediction, error fixing, and syntax generation without leaving the terminal.

Phase 4: Embedded & IoT Toolkit
Native shell commands for serial monitoring, network IP discovery, MQTT testing, and MCU compiling/flashing without external IDEs.

Phase 5: GUI Terminal & OS Integration
GPU-accelerated rendering and integration as the default system shell and desktop environment for a custom Linux distribution.
