/* 

this file contains functions for extra purposes
for example gethostname or user name etc.

*/

#include "extras.h"
#include <unistd.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define INITIAL_BUFSIZE 64


char *getusername(void) {
    struct passwd *pw;
    uid_t uid;
    uid = geteuid();
    pw = getpwuid(uid);

    if (pw) {
        return pw->pw_name;
    } return NULL;
}

char *getdevicename(void) {

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


int isnumeric(char *num) {
    int length = strlen(num);
    for (int i = 0; i < length; i++) {
        if (!isdigit(num[i])) {
            return 0;  
        }
    }
    return 1;  
}

int is_operator_char(char c) {
    return (c == '&' || c == '|' || c == '<' || c == '>' || c == ';');
}

// her sistemde bu fonksiyon olmayabiliyormuş ama nedenini anlamadım
char *strndup(const char *s, size_t n) {
    char *result = malloc(n + 1);
    if (!result) return NULL;
    strncpy(result, s, n);
    result[n] = '\0';
    return result;
}

// hardest function of my life. needs a huge bug check but i am moving on.
char **shell_parser(char *line) {
    int bufsiz = INITIAL_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsiz * sizeof(char *)); // bir satır maks 64 karakter.
    int i = 0;

    if (!tokens) {
        fprintf(stderr, "esh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (line[i] != '\0') {
        while (isspace(line[i])) i++;   // baştaki boşlukları atlar
        if (line[i] == '\0') break;     // boşluk sonrası ilk gelen karakter null ise promp okunmaz

        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i++];
            int start = i;

            if (quote == '"') {
                while (line[i] && !(line[i] == '"' && line[i - 1] != '\\')) i++;
            } else {
                while (line[i] && line[i] != '\'') i++;
            }

            if (line[i] == '\0') {
                fprintf(stderr, "esh: unmatched quote\n");
                exit(EXIT_FAILURE);
            }

            tokens[position++] = strndup(&line[start], i - start);
            i++;
        }

        else if (line[i] == '&') {
            tokens[position++] = strndup(&line[i], 1);
            i++;
        }

        else {
            int start = i;
            while (line[i] &&
                   !isspace(line[i]) &&
                   line[i] != '&') {
                if (line[i] == '\\' && line[i+1] != '\0') i++;
                i++;
            }
            tokens[position++] = strndup(&line[start], i - start);
        }

        if (position >= bufsiz) {
            bufsiz *= 2;
            tokens = realloc(tokens, bufsiz * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "esh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    tokens[position] = NULL;
    return tokens;
}