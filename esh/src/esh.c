#include "esh.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void esh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf(" $ ");
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
    int bufsiz = 64, position = 0;  // here bufsiz is token num in one line.
    char *token, **tokens = malloc(bufsiz * sizeof(char*));
    // positionda token sırası burda.
  

    if (!tokens) {
        fprintf(stderr, "esh: token buffer allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = token;
        position++;
    
        if (position >= bufsiz) {
          bufsiz += 64;
          tokens = realloc(tokens, bufsiz * sizeof(char*));
          if (!tokens) {
            fprintf(stderr, "esh: allocation error\n");
            exit(EXIT_FAILURE);
          }
        }
    
        token = strtok(NULL, " \n\t\r\a");
        while(token != NULL) {
            tokens[position] = token;
            position++;
            if (position >= bufsiz) {
                bufsiz += bufsiz;
                tokens = realloc(tokens, bufsiz * sizeof(char*));
                if (!tokens) {
                    fprintf(stderr, "esh: token buffer allocation error\n");
                    exit(EXIT_FAILURE);
                }
            }
            token = strtok(NULL, " \n\t\r\a");
            // strtok kullanımı böyle ilk çağrıda cümle ikinci çağrıda null verilir devam eder.
        }
        tokens[position] = NULL;
        return tokens;

    }
}


// starts the process
int esh_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();

    if (pid == 0) {
        // child process
        if (execvp(args[0], args) == -1) {
            perror("esh: execvp error");
        }
        exit(EXIT_FAILURE);
    } 
    
    else if (pid < 0) {
        perror("esh: fork process error");
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
    "exit"
};
  
int esh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "esh: expected argument to \"cd\"\n");
    } 
    else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
      return 1;
}

int esh_help(char **args) {
    int i;
    printf("ege's shell - esh\n");
    printf("built ins: \n");
  
    for (i = 0; i < builtin_num(); i++) {
      printf("  %s\n", builtin_str[i]);
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
    &esh_exit
};

int builtin_num(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

int esh_execute(char **args) {
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < builtin_num(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return esh_launch(args);
}