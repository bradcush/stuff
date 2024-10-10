#include <stddef.h>

#ifndef LIST_OPTIONS_H
#define LIST_OPTIONS_H

// Options with defaults set
// during program initialization
typedef struct list_opts {
  int hflag;
} list_opts_t;

int set_list_options(int argc, char **argv, list_opts_t *opts, int *subind);

void print_list_options(int argc, char **argv, list_opts_t *opts);

#endif
