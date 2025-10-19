<h1>MiniShell </h1>

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


## Compilation & Example sessions

To build and run the shell:
```bash
gcc myshell.c -o myshell
./myshell

myshell> ls -l | grep .c > out.txt
myshell> cat < input.txt | sort | uniq
myshell> sleep 10 &
myshell> cd /home && pwd
myshell> exit

```

## Known Limitations & Future Work

### Limitations
- Does *not* currently support logical operators such as `&&` or `||`
- No handling of quoted strings or escape sequences (e.g., `"hello world"`) in the tokenizer
- Limited job-control: no `jobs`, `fg`, `bg` built-ins or process-group management
- Minimal error handling for malformed inputs or extreme edge cases
- Background commands are not tracked in a job list and reaping of background children is minimal

### Future Enhancements
- Implement `history`, `jobs`, `fg`, and `bg` built-ins for full job control
- Support quoting, escaping, and variable expansion in command parsing
- Replace `signal()` with `sigaction()` for more robust signal handling
- Use `waitpid()` with `WNOHANG` to properly reap background children and avoid zombies
- Introduce a dedicated parsing module that supports command chaining (e.g., `&&`, `||`)
- Add a test suite for parsing logic, redirection, and job handling

