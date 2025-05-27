#include <stddef.h>

#ifndef UNLINK_OPTIONS_H
#define UNLINK_OPTIONS_H

// Options with defaults set
// during program initialization
typedef struct unlink_opts {
  int hflag;
} unlink_opts_t;

int set_unlink_options(int argc, char **argv, unlink_opts_t *opts, int *subind);

void print_unlink_options(int argc, char **argv, unlink_opts_t *opts);

#endif
