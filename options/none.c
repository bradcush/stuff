#include "none.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Setting of options when the program starts based
 * on command-line arguments without a command
 */
int set_none_options(int argc, char **argv, none_opts_t *opts) {
  int option;
  const char *short_opt = "dhvt:";
  // Allows handling for single characters
  // debug option is a hidden global
  struct option long_opt[] = {
      {"debug", no_argument, NULL, 'd'},
      {"help", no_argument, NULL, 'h'},
      {"version", no_argument, NULL, 'v'},
      {"test", required_argument, NULL, 't'},
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
      case 'v':
        opts->vflag = 1;
        break;
      case 't': {
        char *tvalue;
        sscanf(
            optarg,
            // What's supported
            "%m[ABC]",
            &tvalue
        );
        // We should only have a value
        // given constraints above
        if (tvalue) {
          opts->tvalue = tvalue;
        } else {
          fprintf(stderr, "Invalid -t argument `%s`.\n", optarg);
          return 1;
        }
        break;
      }
      case '?':
        if (optopt == 't') {
          fprintf(stderr, "Option -%c requires an argument.\n", optopt);
        } else if (isprint(optopt)) {
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
        } else {
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
        }
        return 1;
      default:
        abort();
    }
  }
  return 0;
}

/**
 * Printing to ensure correctness
 */
void print_none_options(int argc, char **argv, none_opts_t *opts) {
  printf(
      "hflag = %d, vflag = %d, tvalue = %s\n",
      opts->hflag,
      opts->vflag,
      opts->tvalue
  );
  for (int index = optind; index < argc; index++) {
    printf("Non-option argument %s\n", argv[index]);
  }
}
