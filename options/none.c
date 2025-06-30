#include "none.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Setting of options when the program starts based
 * on command-line arguments without a command
 */
int set_none_options(int argc, char **argv, none_opts_t *opts, int *subind) {
  int option;
  const char *short_opt = "dhvr:";
  // Allows handling for single characters
  // debug option is a hidden global
  struct option long_opt[] = {
      {"debug", no_argument, NULL, 'd'},
      {"help", no_argument, NULL, 'h'},
      {"version", no_argument, NULL, 'v'},
      {"root", required_argument, NULL, 'r'},
      {NULL, 0, NULL, 0}
  };
  while ((option = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
    switch (option) {
      case 'd':
      case 'r':
      case '?': {
        // Ignore hidden debug, root, and ? for errors.
        // Come back to see if we shouldn't ignore errors.
        break;
      }
      case 'h':
        opts->hflag = 1;
        break;
      case 'v':
        opts->vflag = 1;
        break;
      default:
        abort();
    }
  }
  *subind = optind;
  return 0;
}

/**
 * Printing to ensure correctness
 */
void print_none_options(int argc, char **argv, none_opts_t *opts) {
  printf("hflag = %d\n", opts->hflag);
  printf("vflag = %d\n", opts->vflag);
  for (int index = optind; index < argc; index++) {
    printf("Non-option argument %s\n", argv[index]);
  }
}
