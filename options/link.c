#include "link.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Setting of options when the program starts based on
 * command-line arguments given the link command
 */
int set_link_options(int argc, char **argv, link_opts_t *opts, int *subind) {
  int option;
  const char *short_opt = "dh";
  // Allows handling for single characters
  // debug option is a hidden global
  struct option long_opt[] = {
      {"debug", no_argument, NULL, 'd'},
      {"help", no_argument, NULL, 'h'},
      {NULL, 0, NULL, 0}
  };
  while ((option = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
    switch (option) {
      case 'd':
        // Ignore hidden debug
        break;
      case 'h':
        opts->hflag = 1;
        break;
      case '?':
        if (isprint(optopt)) {
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        } else {
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        }
        return 1;
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
void print_link_options(int argc, char **argv, link_opts_t *opts) {
  printf("hflag = %d\n", opts->hflag);
  for (int index = optind; index < argc; index++) {
    printf("Non-option argument %s\n", argv[index]);
  }
}
