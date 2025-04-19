#include "esh.h"
#include "extras.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// bir struct ile process bilgisi tutulabilir.
typedef struct deviceinfo {
    char *username;
    char *devicename;
} devinfo_t;

void esh_loop(void) {
    char *line;
    char **args;
    int status;

    devinfo_t *info;
    info = (devinfo_t *)malloc(sizeof(devinfo_t));
    info->devicename = getdevicename();
    info->username = getusername();

    do {
        printf("[%s@%s]? ", info->username, info->devicename);  //todo after @ add computer name + working directory
        line = esh_read_line();
        args = esh_split_line(line);
        status = esh_execute(args);
        /*
            yaptığımız iş aslında bütün satırı bir array olarak okmak
            ondan sonra satırdaki her argumanı ayırmak.
            yani line bir uzun satır iken args ise boşluk ile ayrılmış
            elemanları.
        */
        free(line);
        free(args);
    } while (status);

}

char *esh_read_line(void) {
    int bufsiz = 1024;
    int c, position = 0;
    char *buffer = malloc(sizeof(char) * bufsiz);

    if (buffer == NULL) {
        fprintf(stderr, "esh: line buffer allocation error\n");
        exit(EXIT_FAILURE);
    }
    while (1) {
        c = getchar();
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c; // c ya da yeni satır değilse yaz
        } position ++; // while loop her bir karakter için çalıştığı için
        // arttırma işlemi dışarda olmalı yoksa if için \0 karakteri için
        // pozisyon artmaz

        if (position >= bufsiz) {
            bufsiz += 1024;
            buffer = realloc(buffer, bufsiz);  // eski bufferı da kopyalar.
            if (!buffer) {
                fprintf(stderr, "esh: line buffer allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **esh_split_line(char *line) {
    int bufsiz = 64, position = 0;
    char *token, **tokens = malloc(bufsiz * sizeof(char*));

    if (!tokens) {
        fprintf(stderr, "esh: token buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsiz) {
            bufsiz += bufsiz;
            tokens = realloc(tokens, bufsiz * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "esh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL; 
    return tokens;
}

// starts the process
int esh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();

    if (pid == 0) {
        // child process
        if (execvp(args[0], args) == -1) {
            perror("esh");
        }
        exit(EXIT_FAILURE);
    } 
    
    else if (pid < 0) {
        perror("esh");
    }

    // parent process in cocuk process beklemesiyle ilgili bir kod.
    else {
        // parent process işleri
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
            // child işleminin bitmesini bekler

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}

// shell builtin fonksiyonlarımız
char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "builtins"
};

char *builtin_help[] = {
    "cd - takes 1 argument and it is the path you can use '.', '..', '~', ''(i mean no second arg)",
    "help - help or help <builtin> only 1 argument",
    "exit - closes the shell",
    "builtins - shows all the builtins"
};
  
int esh_cd(char **args) {
    const char *target_dir = ".";  // varsayılan olarak current directory

    int arg_count = 0;
    while (args[arg_count] != NULL) {
        arg_count++;
    }
    if (arg_count > 2) {
        fprintf(stderr, "esh: too many arguments\n");
        return 1;
    }


    if (args[1] == NULL) {
        target_dir = "."; // cd yalnızca yazılmışsa mevcut dizinde kal
    } 
    else if (strcmp(args[1], "~") == 0) {
        char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "esh: HOME environment variable not set.\n");
            return 1;
        }
        target_dir = home;
    }
    else {
        target_dir = args[1]; // normal cd path
    }

    if (chdir(target_dir) != 0) {
        perror("esh");
    }

    return 1;
}

int esh_help(char **args) {

    int arg_count = 0;
    while (args[arg_count] != NULL) {
        arg_count++;
    }

    if (arg_count == 1) {
        int i;
        printf("ege's shell - esh\n");
        printf("a basic shell implementation in c by ege cagan kantar");
        
        printf("manual for builtins: help 'builtin'\n");
        printf("help for other functions: man 'function'\n");
        printf("show built ins: <builtins>\n");

    } else if (arg_count == 2) {
        char *funcname = args[1];
        int found = 0;

        for (int i = 0; i < builtin_num(); i++) {
            if (strcmp(funcname, builtin_str[i]) == 0) {
                printf("%s\n", builtin_help[i]);
                found = 1;
                break;
            }
        }

        if (!found) {
            fprintf(stderr, "esh: no help available for '%s'\n", funcname);
            printf("try man <command>\n");
        }
    } else {
        fprintf(stderr, "esh: too many arguments\n");
        return 1;
    }
  
    return 1;
}

int esh_builtins(char **args) {
    int arg_count = 0;
    while (args[arg_count] != NULL) {
        arg_count++;
    }

    if (arg_count > 1 || strcmp(args[0], "builtins") != 0) {
        fprintf(stderr, "esh: wrong usage of command\n");
        printf("correct usage: builtins\n");
        return 1;
    }

    printf("built-in's:\n");
    for (int i = 0; i < builtin_num(); i++) {
        printf(" - %s\n", builtin_str[i]);
    }
    return 1;
}


int esh_exit(char **args) {
    return 0;
}

// bu decleration array of function pointers which take char ** as parameter and return int demektir.
int (*builtin_func[]) (char **) = {
    &esh_cd,
    &esh_help,
    &esh_exit,
    &esh_builtins
};

int builtin_num(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

int esh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    // boş komut ta devam için
    return 1;
  }

  for (i = 0; i < builtin_num(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return esh_launch(args);
}