#include <stddef.h>

#ifndef LIST_OPTIONS_H
#define LIST_OPTIONS_H

// List command options
typedef struct {
  int hflag;
  int lflag;
  int oflag;
} list_opts_t;

int set_list_options(int argc, char **argv, list_opts_t *opts, int *subind);

void print_list_options(int argc, char **argv, list_opts_t *opts);

#endif
