
#include <stdio.h>

#ifndef INTERNAL_FUNCTIONS_H
#define INTERNAL_FUNCTIONS_H

char **shell_parser      (char *);

char **get_builtin_str   (void);
char **get_builtin_help  (void);
char  *get_user_name     (void);
char  *get_device_name   (void);
char  *getcwd_path       (void);

int    is_numeric         (char *);
int    is_operator_char   (char);

char *custom_strndup(const char *s, size_t n);


#endif // INTERNAL_FUNCTIONS_H