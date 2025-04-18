#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

int main(int argc, char *argv[]) {

    // istersen arguman[1] olarak dosya adı al ben projelik olsun diye uğraşmadım

    /*
    if (argc != 2) {
        printf("only add file.");
        return 1;
    }
    */

    char *bigfuck = "src/esh.c";
    FILE *input = fopen(bigfuck, "r");

    // FILE *input = fopen(argv[1], "r");

    if (input == NULL) {
        perror("input file open error");
        return 1;
    }

    FILE *output = fopen("todos.txt", "w+");
    if (output == NULL) {
        perror("output file open error");
        fclose(input);
        return 1;
    }

    char line[MAX_LINE];
    int in_multiline_todo = 0;
    int line_number = 0;
    int todo_start_line = 0;

    while (fgets(line, MAX_LINE, input) != NULL) {
        line_number++;

        if (in_multiline_todo) {
            char *end_comment = strstr(line, "*/");
            if (end_comment != NULL) {

                size_t prefix_len = end_comment - line;
                char before_end[MAX_LINE];
                strncpy(before_end, line, prefix_len);
                before_end[prefix_len] = '\0';

                if (strspn(before_end, " \t\n") != strlen(before_end)) {
                    fprintf(output, "%s\n", before_end);
                }

                in_multiline_todo = 0;
            } else {
                fprintf(output, "%s", line);
            }
            continue;
        }

        char *single = strstr(line, "//todo");
        if (single != NULL) {
            single += 7;
            while (*single == ':' || *single == ' ' || *single == '\t') single++;
            fprintf(output, "%d: %s", line_number, single);
        }

        else {
            char *multi = strstr(line, "/*todo");
            if (multi != NULL) {
                multi += 7;
                while (*multi == ':' || *multi == ' ' || *multi == '\t') multi++;
                fprintf(output, "%d: %s", line_number, multi);

                if (strstr(line, "*/") == NULL) {
                    in_multiline_todo = 1;
                }
            }
        }
    }

    printf("todo's are in written!\n");

    fclose(input);
    fclose(output);
    return 0;
}
