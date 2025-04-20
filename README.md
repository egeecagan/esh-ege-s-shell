# 🐚 esh - Ege's Shell

`esh` (Ege's Shell) is a simple shell implementation written in **C** to explore how command-line interpreters work under the hood.  
This project also includes several built-in features developed for learning and experimentation purposes.

---

## 🚀 How to Run

1. Navigate to the `build` directory:

    ```bash
    cd build
    ```

2. Generate the build files using CMake:

    ```bash
    cmake ..
    ```

3. Compile and run the shell:
    ```bash
    make run
    ```

---

## 🔧 Features

-   Basic command execution (`ls`, `cat`, `echo`, etc.)
-   Command history tracking
-   `!!` to repeat the last executed command
-   Built-in commands:
    -   `cd`
    -   `help`
    -   `exit`
    -   `builtins`
    -   `history [num]` (optional argument)

> Type `builtins` to see all built-in commands.  
> Use `help <builtin>` for more information on a specific command.

---

## 📌 Planned Features (To Be Implemented maybe)

-   Signal handling (e.g., `Ctrl+C`)
-   Piping (`|`)
-   Stream redirection (`>`, `<`, `>>`)
-   Reading configuration files at startup
-   `!num` to execute a specific history command
-   Background process support (`&`)

---

## 📝 Todo Checker

There is also a small helper tool called `todo` included in the project.  
It checks the ``file for`TODO` tags to remind what’s left to implement.

### Usage:

```bash
./todo <filename>
```

> You can also pass a file name as an argument to check a different file:
>
> ```bash
> ./todo src/another_file.c
> ```

---

## 📚 Purpose

This is a **learning project**.  
The main goal is to understand how a shell interacts with the OS using system calls like `fork()`, `execvp()`, `wait()`, and handle things like input parsing, memory management, and process control manually.

---

Made with ☕ and `C` by **Ege Çağan Kantar**
