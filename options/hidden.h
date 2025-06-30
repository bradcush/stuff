#include <stddef.h>

#ifndef HIDDEN_OPTIONS_H
#define HIDDEN_OPTIONS_H

// Top-level hidden debug options
typedef struct {
  int dflag;
  char *rvalue;
} hidden_opts_t;

extern hidden_opts_t ghidden_opts;

int set_hidden_options(int argc, char **argv, hidden_opts_t *opts, int *subind);

#endif
