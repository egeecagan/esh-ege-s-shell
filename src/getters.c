#include <unistd.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal_functions.h"

static char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "builtins",
    "history"
};

static char *builtin_help[] = {
    "cd - takes 1 argument and it is the path you can use '.', '..', '~', ''(i mean no second arg)",
    "help - help or help <builtin> only 1 argument",
    "exit - closes the shell",
    "builtins - shows all the builtins",
    "history <num> -> optional - displays the last <num> commands executed succsesfully. if num not given it will print only 10"
};

char **get_builtin_str(void) {
    return builtin_str;
}

char **get_builtin_help(void) {
    return builtin_help;
}

char *get_user_name(void) {
    struct passwd *pw;
    uid_t uid;
    uid = geteuid();
    pw = getpwuid(uid);

    if (pw) {
        return pw->pw_name;
    } return NULL;
}

char *get_device_name(void) {

    const char *command = "scutil --get ComputerName";
    FILE *f = popen("scutil --get ComputerName", "r");

    const char *unk = "unknown";
    char *device = (char *)malloc(sizeof(char) * 256);

    if (!device) {
        if (f != NULL) {
            pclose(f);
        }
        return strdup(unk);
                            
    }

    if (!f) {
        strncpy(device, unk, 256);
    } else {
        if (fgets(device, 256, f) == NULL) {
            strncpy(device, unk, 256);
            device[255] = '\0';
        } else {
            device[strcspn(device, "\r\n\t")] = '\0';  
        }
        pclose(f);
    }

    return device;
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