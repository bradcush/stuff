#include <stddef.h>

#ifndef INIT_OPTIONS_H
#define INIT_OPTIONS_H

// Options with defaults set
// during program initialization
typedef struct init_opts {
  int hflag;
} init_opts_t;

int set_init_options(int argc, char **argv, init_opts_t *opts, int *subind);

void print_init_options(int argc, char **argv, init_opts_t *opts);

#endif
