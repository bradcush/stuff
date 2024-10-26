#include <stddef.h>

#ifndef LINK_OPTIONS_H
#define LINK_OPTIONS_H

// Options with defaults set
// during program initialization
typedef struct link_opts {
  int hflag;
} link_opts_t;

int set_link_options(int argc, char **argv, link_opts_t *opts, int *subind);

void print_link_options(int argc, char **argv, link_opts_t *opts);

#endif
