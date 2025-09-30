#ifndef ESH_H
#define ESH_H

void esh_loop(void);
char *esh_read_line(void);
char **esh_split_line(char *);
int esh_launch(char **);

char *getcwd_path(void);

int esh_cd(char **);
int esh_help(char **);
int esh_builtins(char **);
int esh_history(char **);
int esh_exit(char **);

int builtin_num(void);
int esh_execute(char **, char **);

#endif //ESH_H