
#ifndef ESH_H
#define ESH_H

void esh_loop(void);
char *esh_read_line(void);
char **esh_split_line(char *);
int esh_launch(char **);

int esh_cd(char **);
int esh_help(char **);
int esh_builtins(char **);
int esh_exit(char **);

int builtin_num(void);
int esh_execute(char **);

#endif //ESH_H