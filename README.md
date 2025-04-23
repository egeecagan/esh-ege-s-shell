# 🐚 esh - ege's shell

`esh` (ege's shell) is a simple shell implementation written in **C** to explore how command-line interpreters work under the hood.  
this project also includes several built-in features developed for learning and experimentation purposes.


## 🚀 how to run
create a build folder ind the esh folder and then
```bash
cd build
cmake ..
make run
```

## features

-   basic command execution (`ls`, `cat`, `echo`, etc.)
-   command history tracking
-   `!!` to repeat the last executed command
-   built-in commands:
    -   `cd`
    -   `help`
    -   `exit`
    -   `builtins`
    -   `history [num]` (optional argument)
- also in last commit i implemented signal handling now you can interrupt a process by using ctrl + c it will not stop the main shell process.

> type `builtins` to see all built-in commands.  
> use `help <builtin>` for more information on a specific command.


## 📌 planned features (to be implemented maybe)

-   piping `|`
-   stream redirection `>`, `<`, `>>`
-   reading configuration files at startup
-   `!num` to execute a specific history command
-   Background process support `&`
-   up and down keys to get last executed command directly
so basically the operators and `;`are not implemented yet.

## 📝 todo checker

there is also a small helper tool called `todo` included in the project.  
it checks the file for `TODO` tags to remind what’s left to implement.

there is a mini bug i know with this if you create multi line command and
use onlt one line it will print the line plus the last star and slash.

### usage:

```bash
./todo <filename>
```

## 📚 references & inspiration

this project was inspired by the guide ["Write a Shell in C"](https://brennan.io/2015/01/16/write-a-shell-in-c/) by Stephen Brennan.
it served as a foundational resource to understand how to implement basic shell features in C. 
it's a really well-written and highly educational article — i strongly recommend it to anyone interested in how shell is working.

Made with `C` by **Ege Çağan Kantar**
