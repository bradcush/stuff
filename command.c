#include "command.h"
#include "options/hidden.h"
#include "options/list.h"
#include "options/none.h"
#include <fnmatch.h>
#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *CURRENT_DIRECTORY = ".";

/**
 * Map a command to a better suited enum
 */
command_t map_command(char *command) {
  const struct {
    command_t val;
    const char *str;
  } map[] = {{LIST, "list"}, {NONE, ""}};
  size_t length = sizeof(map) / sizeof(map[0]);
  for (int i = 0; i < length; i++) {
    if (!strcmp(command, map[i].str)) {
      return map[i].val;
    }
  }
  // Everything else is deemed invalid but once we
  // accept arguments we'll have to be more rigorous
  fprintf(stderr, "Invalid command `%s`\n", command);
  exit(EXIT_FAILURE);
}

/**
 * Print help information on how to use
 * command-line flags and accepted arguments
 */
void print_none_usage(char **argv) {
  printf("Usage: %s <command> [options]\n\n", argv[0]);
  printf("Command-line dotfiles management\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n");
  printf("  -v, --version        Print the current version number\n");
  printf("  -t, --test           Test flags accepting arguments\n\n");
}

/**
 * Print help information for list command
 * command-line flags and accepted arguments
 */
void print_list_usage(char **argv) {
  printf("Usage: %s list [options]\n\n", argv[0]);
  printf("List all tracked dotfiles\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n\n");
}

/**
 * Handle a directory entry by filtering ignored
 * directories and logging the rest to stdout
 */
int treat_entry(const char *fpath, const struct stat *sb, int tflag) {
  // Hardcoding hidden git directory
  char *pattern = "!(./.git*)";
  int flags = FNM_EXTMATCH;
  if (fnmatch(pattern, fpath, flags) == 0) {
    printf("%s\n", fpath);
  }
  return 0;
}

/**
 * Handle NONE command
 */
void treat_none(int argc, char **argv, hidden_opts_t *hopts) {
  none_opts_t opts = {0, 0, NULL};
  char default_mode = 'A';
  opts.tvalue = &default_mode;
  if (set_none_options(argc, argv, &opts) != 0) {
    fprintf(stderr, "Failure setting none options\n");
    exit(EXIT_FAILURE);
  }
  if (hopts->dflag) {
    print_none_options(argc, argv, &opts);
  }
  // We give priority to certain options
  // and stop executing depending
  if (opts.hflag) {
    print_none_usage(argv);
    exit(EXIT_SUCCESS);
  }
  if (opts.vflag) {
    // Hardcoded version for now
    printf("stuff version %s\n", "0.0.1");
  }
  // Best to call usage explicitly
  // when no arguments given
  if (argc == 1) {
    print_none_usage(argv);
  }
}

/**
 * Handle LIST command
 */
void treat_list(int argc, char **argv, hidden_opts_t *hopts) {
  list_opts_t opts = {0};
  // Not throwing for invalid subcommand
  if (set_list_options(argc, argv, &opts) != 0) {
    fprintf(stderr, "Failure setting list options\n");
    exit(EXIT_FAILURE);
  }
  if (hopts->dflag) {
    print_list_options(argc, argv, &opts);
  }
  // We give priority to certain options
  // and stop executing depending
  if (opts.hflag) {
    print_list_usage(argv);
    exit(EXIT_SUCCESS);
  }
  // Walk the file tree and handle entries
  if (ftw(CURRENT_DIRECTORY, treat_entry, 20) == -1) {
    fprintf(stderr, "Error walking directory\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * Handle functionality specific to a command or lack thereof
 */
void treat_command(char *command, int argc, char **argv, hidden_opts_t *hopts) {
  switch (map_command(command)) {
    case NONE:
      treat_none(argc, argv, hopts);
      break;
    case LIST:
      treat_list(argc, argv, hopts);
      break;
    default:
      fprintf(stderr, "Unreachable treat_command\n");
      exit(EXIT_FAILURE);
  }
}
