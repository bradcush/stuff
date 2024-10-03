#include <stddef.h>

#ifndef HIDDEN_OPTIONS_H
#define HIDDEN_OPTIONS_H

// Options with defaults set
// during program initialization
typedef struct hidden_opts {
  int dflag;
} hidden_opts_t;

int set_hidden_options(int argc, char **argv, hidden_opts_t *opts, int *subind);

#endif
