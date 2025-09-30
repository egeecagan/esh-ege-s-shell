

#ifndef TYPES_H
#define TYPES_H



typedef struct device_info {
    char *user_name;
    char *device_name;
} device_info_t;

typedef struct hist_node {
  struct hist_node *next;
  char *command;  
} hist_node_t;

typedef struct cmd_hist {
    hist_node_t *front;
    hist_node_t *rear;
} cmd_hist_t;

extern cmd_hist_t *hist;
extern int hist_count;

#endif