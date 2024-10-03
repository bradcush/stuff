#include "hidden.h"
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Setting of global hidden options used for debugging
 * which should be ignored by other option parsers
 */
int set_hidden_options(
    int argc, char **argv, hidden_opts_t *opts, int *subind
) {
  int option;
  // Disable errors globally
  // for hidden options
  opterr = 0;
  const char *short_opt = "d";
  // Allows handling for single characters
  struct option long_opt[] = {
      {"debug", no_argument, NULL, 'd'}, {NULL, 0, NULL, 0}
  };
  while ((option = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
    switch (option) {
      case 'd':
        opts->dflag = 1;
        break;
      case '?':
        // Ignore other options
        break;
      default:
        abort();
    }
  }
  opterr = 1;
  *subind = optind;
  // Reset option index
  optind = 1;
  return 0;
}
