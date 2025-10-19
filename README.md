<h1>Custom Unix Shell </h1>

A lightweight, Unix-like shell written in C, designed to demonstrate core systems programming concepts including process control, I/O redirection, piping, and signal handling. Built for clarity, extensibility, and recruiter visibility.

## Features

- **Command Execution**  
  Executes standard Linux commands using `fork()` and `execvp()`.

- **Piping (`|`)**  
  Supports multi-stage pipelines across up to 10 commands using `pipe()` and `dup2()`.

- **I/O Redirection**

  - `>`: Redirect stdout to file (overwrite)
  - `>>`: Redirect stdout to file (append)
  - `<`: Redirect stdin from file

- **Background Execution (`&`)**  
  Runs commands asynchronously without blocking the shell loop.

- **Built-in Commands**

  - `cd`: Change working directory
  - `exit`: Gracefully terminate the shell

- **Signal Handling**  
  Custom `SIGINT` handler prevents accidental termination via `Ctrl+C`, prompting user to use `exit`.

- **Robust Parsing**  
  Tokenizes input into commands and arguments with support for whitespace, redirection, and background flags.

## Technical Highlights

| Concept                         | Implementation Insight                                                                  |
| ------------------------------- | --------------------------------------------------------------------------------------- |
| **Process Control**             | Uses `fork()` to spawn child processes and `execvp()` for command execution             |
| **Inter-process Communication** | Implements `pipe()` and `dup2()` to connect stdout/stdin across piped commands          |
| **File Descriptor Management**  | Handles redirection with `open()`, `dup2()`, and `close()` for clean stream control     |
| **Signal Safety**               | Overrides default `SIGINT` behavior to maintain shell continuity                        |
| **Modular Design**              | Separates parsing, redirection, background detection, and execution for maintainability |
