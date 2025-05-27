#include <stddef.h>

#ifndef NONE_OPTIONS_H
#define NONE_OPTIONS_H

// Commandless options
typedef struct none_opts {
  int hflag;
  int vflag;
  char *tvalue;
} none_opts_t;

int set_none_options(int argc, char **argv, none_opts_t *opts, int *subind);

void print_none_options(int argc, char **argv, none_opts_t *opts);

#endif
