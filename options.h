#include <stddef.h>

#ifndef OPTIONS_H
#define OPTIONS_H

// Options with defaults set
// during program initialization
typedef struct opts_t {
  int hflag;
  int vflag;
  char *tvalue;
} opts_t;

int set_options(int argc, char **argv, opts_t *opts);

void print_options(int argc, char **argv, opts_t *opts);

#endif
