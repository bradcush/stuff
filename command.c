#include "command.h"
#include "options/hidden.h"
#include "options/list.h"
#include "options/none.h"
#include <fnmatch.h>
#include <ftw.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Printing colored output
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

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
  return NONE;
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
 * Read stats for a given file
 */
void get_file_stats(char *filename, struct stat *sb) {
  if (stat(filename, sb) == -1) {
    // Clean way of outputting errno
    perror("unknown stat issue");
    exit(EXIT_FAILURE);
  }
  // We only want to handle certain files
  // directory, symlink, regular file
  int mode = sb->st_mode & S_IFMT;
  switch (mode) {
    case S_IFDIR:
    case S_IFLNK:
    case S_IFREG:
      // Allowed list
      break;
    default:
      fprintf(stderr, "Unsupported file mode `%d'", mode);
      exit(EXIT_FAILURE);
  }
}

/**
 * Read stats for an expected link which often
 * won't exist but we want to handle nicely
 */
int get_link_stats(char *filename, struct stat *sb) {
  if (lstat(filename, sb) == -1) {
    // Assuming no such file or directory
    return 1;
  }
  // We only want to handle certain files
  // directory, symlink, regular file
  int mode = sb->st_mode & S_IFMT;
  switch (mode) {
    case S_IFDIR:
    case S_IFLNK:
    case S_IFREG:
      // Allowed list
      break;
    default:
      fprintf(stderr, "Unsupported file mode `%d'", mode);
      exit(EXIT_FAILURE);
  }
  return 0;
}

/**
 * Make the system path for some local path where a first version
 * doesn't do special mapping. Allocates memory for the returned
 * pointer which needs to be freed by the caller.
 */
char *make_system_path(const char *fpath) {
  // Just removing the expected dot for now
  int path_size = strlen(fpath) * sizeof(char);
  char *spath = (char *)malloc(path_size);
  return strcpy(spath, ++fpath);
}

/**
 * Handle a directory entry by filtering ignored
 * directories and logging the rest to stdout
 */
int treat_entry(const char *fpath, const struct stat *sb, int tflag) {
  // Hardcoding hidden git and current directory
  // but should move to persisted file input
  char *pattern = "!(./.git*|.)";
  int flags = FNM_EXTMATCH;
  if (fnmatch(pattern, fpath, flags) == 0) {
    struct stat fsb, lsb;
    // Not doing anything with these stats
    get_file_stats((char *)fpath, &fsb);
    char *spath = make_system_path(fpath);
    int errlink = get_link_stats((char *)spath, &lsb);
    char *spathnorm = errlink ? "x" : spath;
    // Just print the number of links and
    // highlight them in green for now
    if (!errlink) {
      printf(
          ANSI_COLOR_GREEN "%3ju %s -> %s\n" ANSI_COLOR_RESET,
          (uintmax_t)lsb.st_nlink,
          fpath,
          spathnorm
      );
    } else {
      printf("%3ju %s -> %s\n", (uintmax_t)0, fpath, spathnorm);
    }
    free(spath);
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
  int subind = 0;
  if (set_none_options(argc, argv, &opts, &subind) != 0) {
    fprintf(stderr, "Failure setting options\n");
    exit(EXIT_FAILURE);
  }
  if (hopts->dflag) {
    print_none_options(argc, argv, &opts);
  }
  // Not supporting non-options
  if (subind < argc) {
    fprintf(stderr, "Invalid non-option `%s'\n", argv[subind]);
    exit(EXIT_FAILURE);
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
  int subind = 0;
  // Not throwing for invalid subcommand
  if (set_list_options(argc, argv, &opts, &subind) != 0) {
    fprintf(stderr, "Failure setting list options\n");
    exit(EXIT_FAILURE);
  }
  if (hopts->dflag) {
    print_list_options(argc, argv, &opts);
  }
  // Current should be LIST so next
  // is invalid if within limit
  if (++subind < argc) {
    fprintf(stderr, "Invalid list non-option `%s'\n", argv[subind]);
    exit(EXIT_FAILURE);
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
