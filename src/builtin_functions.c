#include "types.h"
#include "internal_functions.h"
#include "esh.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int esh_builtins(char **args) {
    char **builtin_str = get_builtin_str();

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

        hist_node_t *cmd = hist->front;

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
        if (is_numeric(args[1])) {
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
    
            hist_node_t *cmd = hist->front;
    
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
    hist_node_t *cmd = hist->front;

    while (cmd != NULL) {
        hist_node_t *next = cmd->next;
        free(cmd->command);  
        free(cmd);          
        cmd = next;         
    }

    free(hist);  
    return 0;   
}


int esh_help(char **args) {

    char **builtin_str = get_builtin_str();
    char **builtin_help = get_builtin_help();

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

int builtin_num(void) {
    char **builtin_str = get_builtin_str();
    return sizeof(builtin_str) / sizeof(char *);
}