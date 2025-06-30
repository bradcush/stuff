#include <stddef.h>

#ifndef NONE_OPTIONS_H
#define NONE_OPTIONS_H

// Commandless options
typedef struct {
  int hflag;
  int vflag;
} none_opts_t;

int set_none_options(int argc, char **argv, none_opts_t *opts, int *subind);

void print_none_options(int argc, char **argv, none_opts_t *opts);

#endif
