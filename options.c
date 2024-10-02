#include "options.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * Print help information on how to use
 * command-line flags and accepted arguments
 */
void print_usage(char **argv) {
  printf("Usage: %s [options]\n\n", argv[0]);
  printf("Command-line dotfiles management\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n");
  printf("  -v, --verbose        Print verbose logging output\n");
  printf("  -t, --test           Test flags accepting arguments\n\n");
}

/**
 * Setting of options when the program
 * starts based on command line arguments
 */
int set_options(int argc, char **argv, opts_t *opts) {
  int option;
  const char *short_opt = "hlvt:";
  // Allows handling for single characters
  struct option long_opt[] = {
      {"help", no_argument, NULL, 'h'},
      {"verbose", no_argument, NULL, 'v'},
      {"test", required_argument, NULL, 't'},
      {NULL, 0, NULL, 0}
  };
  while ((option = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1) {
    switch (option) {
      case 'h':
        opts->hflag = 1;
        print_usage(argv);
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
  // Print remaining not options
  if (optind < argc) {
    printf("non-option ARGV-elements: ");
    while (optind < argc) {
      printf("%s ", argv[optind++]);
    }
    putchar('\n');
  }
  return 0;
}

/**
 * Printing to ensure corrrectness
 */
void print_options(int argc, char **argv, opts_t *opts) {
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
