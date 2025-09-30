#include "esh.h"
#include "extras.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

cmd_hist_t *hist;
int hist_count;

void esh_loop(void) {
    signal(SIGINT, SIG_IGN);   
    signal(SIGTTOU, SIG_IGN);  
    signal(SIGTTIN, SIG_IGN);  

    hist = malloc(sizeof(cmd_hist_t));
    if (hist) {
        hist_count = 0;
    } else {
        fprintf(stderr, "history allocation error\n");
    }

    char *line;
    char **args;
    int status;

    char *home = getenv("HOME");
    if (home != NULL) {
        chdir(home);
    } else {
        chdir("/");
    }

    devinfo_t *info;
    info = (devinfo_t *)malloc(sizeof(devinfo_t));
    info->devicename = getdevicename();
    info->username = getusername();

    do {
        char *path = getcwd_path();
        printf("[\033[36m%s\033[0m@\033[35m%s\033[0m %s] \033[32m$\033[0m ", info->username, info->devicename, path);
        free(path);
    
        line = esh_read_line();
    
        if (line == NULL || strlen(line) == 0 || strspn(line, " \t\r\n") == strlen(line)) {
            status = 1;
        }

        else if (strcmp(line, "!!") == 0) {
            if (hist->rear != NULL) {

                char *real_command = strdup(hist->rear->command);        

                int linelen = strlen(real_command);
                node_t *historynode = (node_t *)malloc(sizeof(node_t));
                historynode->command = malloc(sizeof(char) * (linelen + 1));
                strcpy(historynode->command, real_command);
                historynode->next = NULL;
        
                if (hist->front == NULL && hist->rear == NULL) {
                    hist->front = historynode;
                    hist->rear = historynode;
                    hist_count++;
                } else {
                    hist->rear->next = historynode;
                    hist->rear = historynode;
                    hist_count++;
                }
                printf("%s\n",real_command);

                char **prev_args = esh_split_line(real_command);
                status = esh_execute(prev_args);
        
                free(real_command);
                free(prev_args);
            } else {
                fprintf(stderr, "esh: no previous command found\n");
                status = 1;
            }
        }
        else {
            int linelen = strlen(line);

            node_t *historynode = (node_t *)malloc(sizeof(node_t));
            historynode->command = malloc(sizeof(char) * (linelen + 1));

            strcpy(historynode->command, line);
            historynode->next = NULL;
    
            if (hist->front == NULL && hist->rear == NULL) {
                hist->front = historynode;
                hist->rear = historynode;
                hist_count++;
            } else {
                hist->rear->next = historynode;
                hist->rear = historynode;
                hist_count++;
            }
    
            args = esh_split_line(line);
            status = esh_execute(args);
            free(args);
        }

        free(line);
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
            buffer[position] = c; 
        } position ++; 

        if (position >= bufsiz) {
            bufsiz += 1024;
            buffer = realloc(buffer, bufsiz); 
            if (!buffer) {
                fprintf(stderr, "esh: line buffer allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

char **esh_split_line(char *line) {
    char **val = shell_parser(line);

    // printf("tokens :\n");
    // for (int i = 0; val[i] != NULL; i++) {
    //     printf("  [%d]: '%s'\n", i, val[i]);
    // }

    return val;
}

int esh_launch(char **args) {
    pid_t pid;
    int status;
    pid = fork();

    if (pid == 0) {
        setpgid(0, 0);// pgid process group id demek ve bu child processi esh nin olduğu gruptan alır kendi grubunu açar ve lider yapar   
        signal(SIGINT, SIG_DFL);           
        tcsetpgrp(STDIN_FILENO, getpid());    
        execvp(args[0], args);
        perror("esh");
        exit(EXIT_FAILURE);
    } 
    else if (pid < 0) {
        perror("esh");
        return 1;
    } 
    else {
        setpgid(pid, pid);                    
        tcsetpgrp(STDIN_FILENO, pid);         
        waitpid(pid, &status, WUNTRACED);     
        tcsetpgrp(STDIN_FILENO, getpgrp());  

        // suspend olduysa devam ettir (bazı durumlar için)x
        if (WIFSTOPPED(status)) {
            kill(pid, SIGCONT);
        }
    }
    return 1;
}


char *getcwd_path(void) {
    size_t size = 1024;
    char *buffer = (char *)malloc(size);

    const char *fallback = "unknown";

    if (!buffer) {
        fprintf(stderr, "buffer alloc error for getcwd\n");

        return strdup(fallback); 
    } else {
        if (getcwd(buffer, size) == NULL) {
            fprintf(stderr, "buffer alloc error for getcwd\n");
            return strdup(fallback);
        }
        return buffer;
    }
}

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "builtins",
    "history"
};

char *builtin_help[] = {
    "cd - takes 1 argument and it is the path you can use '.', '..', '~', ''(i mean no second arg)",
    "help - help or help <builtin> only 1 argument",
    "exit - closes the shell",
    "builtins - shows all the builtins",
    "history <num> -> optional - displays the last <num> commands executed succsesfully. if num not given it will print only 10"
};
  
int esh_cd(char **args) {
    const char *target_dir = "."; 

    int arg_count = 0;
    while (args[arg_count] != NULL) {
        arg_count++;
    }
    if (arg_count > 2) {
        fprintf(stderr, "esh: too many arguments\n");
        return 1;
    }

    if (args[1] == NULL) {
        target_dir = "."; 
    } 
    else if (strcmp(args[1], "~") == 0) {
        char *home = getenv("HOME");
        if (home == NULL) {
            fprintf(stderr, "esh: home environment variable not set.\n");
            return 1;
        }
        target_dir = home;
    }
    else {
        target_dir = args[1]; 
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
        printf("help for other functions: man <function>\n");
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

int esh_history(char **args) {
    if (hist == NULL || hist->front == NULL) {
        printf("history is empty.\n");
        return 1;
    }
    int arg_count = 0;
    while (args[arg_count] != NULL) {
        arg_count++;
    }
    if (arg_count == 1) {
        int start_index = hist_count > 10 ? hist_count - 10 : 0;
        int current_index = 0;
        int print_index = hist_count > 10 ? hist_count - 10 + 1 : 1;

        node_t *cmd = hist->front;

        while (cmd != NULL) {
            if (current_index >= start_index && cmd->next != NULL) {
                printf("%d - %s\n", print_index, cmd->command);
                print_index++;
            }
            cmd = cmd->next;
            current_index++;
        }

        return 1;
    } 

    else if (arg_count == 2) {
        int num;
        if (isnumeric(args[1])) {
            num = atoi(args[1]);
    
            if (num <= 0) {
                fprintf(stderr, "esh: history argument must be a positive integer\n");
                return 1;
            }
    
            if (num > hist_count) {
                fprintf(stderr, "given num is bigger than hist size last 10 given\n");
                num = 10; 
            }
    
            int skip_last = 1; 
            int visible_count = hist_count - skip_last;
    
            if (num > visible_count) {
                num = visible_count;
            }
    
            int start_index = visible_count - num + 1;
            int current_index = 1;
    
            node_t *cmd = hist->front;
    
            while (cmd != NULL) {
                if (current_index >= start_index && cmd->next != NULL) {
                    printf("%d - %s\n", current_index, cmd->command);
                }
    
                cmd = cmd->next;
                current_index++;
            }
            return 1;
        } 
        else {
            fprintf(stderr, "esh: history takes an integer\n");
            return 1;
            }
        } 
    else {
        fprintf(stderr, "wrong usage correct way either only history or history <num>\n");
        return 1;
    }   
}

int esh_exit(char **args) {
    node_t *cmd = hist->front;

    while (cmd != NULL) {
        node_t *next = cmd->next;
        free(cmd->command);  
        free(cmd);          
        cmd = next;         
    }

    free(hist);  
    return 0;   
}

// bu decleration array of function pointers which take char ** as parameter and return int demektir.
int (*builtin_func[]) (char **) = {
    &esh_cd,
    &esh_help,
    &esh_exit,
    &esh_builtins,
    &esh_history
};

int builtin_num(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

int esh_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < builtin_num(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return esh_launch(args);
}