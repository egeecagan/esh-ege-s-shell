#ifndef ESH_H
#define ESH_H

typedef struct deviceinfo {
    char *username;
    char *devicename;
} devinfo_t;

typedef struct node {
  struct node *next;
  char *command;  
} node_t;

typedef struct cmd_hist {
    node_t *front;
    node_t *rear;
} cmd_hist_t;

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
int esh_execute(char **);

#endif //ESH_H