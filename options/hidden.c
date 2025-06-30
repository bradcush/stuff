#include "hidden.h"
#include <ctype.h>
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
  const char *short_opt = "dfhlovr:";
  // Allows handling for single characters
  struct option long_opt[] = {
      {"debug", no_argument, NULL, 'd'},
      // All subcommand options need to be ignored but this can
      // get tricky because different letters might represent
      // different options across all subcommands
      {"force", no_argument, NULL, 'f'},
      {"help", no_argument, NULL, 'h'},
      {"linked", no_argument, NULL, 'l'},
      {"owner", no_argument, NULL, 'o'},
      {"version", no_argument, NULL, 'v'},
      {"root", required_argument, NULL, 'r'},
      {NULL, 0, NULL, 0}
  };
  while ((option = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
    switch (option) {
      case 'd':
        opts->dflag = 1;
        break;
      case 'f':
      case 'h':
      case 'l':
      case 'o':
      case 'v':
        // Ignore non-hidden options
        break;
      case 'r': {
        if (optarg) {
          opts->rvalue = optarg;
        } else {
          // Seems to be skipping options directly after due
          // to something in hidden options so remove hidden
          // options or arguments accepting values
          fprintf(stderr, "Invalid -r argument `%s'.\n", optarg);
          return 1;
        }
        break;
      }
      case '?':
        if (optopt == 'r') {
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
  opterr = 1;
  *subind = optind;
  // Reset option index
  optind = 1;
  return 0;
}
