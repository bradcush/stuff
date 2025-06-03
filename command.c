#include "command.h"
#include "options/hidden.h"
#include "options/link.h"
#include "options/list.h"
#include "options/none.h"
#include "options/unlink.h"
#include <fnmatch.h>
#include <ftw.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Defining colored output
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define GREEN(str) ANSI_COLOR_GREEN str ANSI_COLOR_RESET

const char *CURRENT_DIRECTORY = ".";

uint USER_FULL = S_IREAD | S_IWRITE | S_IEXEC;

/**
 * Map a command to a better suited enum
 */
command_t map_command(char *command) {
  const struct {
    command_t val;
    const char *str;
  } map[] = {{NONE, ""}, {LINK, "link"}, {LIST, "list"}, {UNLINK, "unlink"}};
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
  printf("stuff is a tool intended to make it simple to manage dotfiles\n"
         "using a repository with version control. There is a list of\n"
         "commands and options below that can be called with the additional\n"
         "`--help' flag for more information.\n\n");
  printf("Commands:\n");
  printf("  link                 Link local files or directories\n");
  printf("  list                 List all tracked dotfiles\n");
  printf("  unlink               Unlink local files or directories\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n");
  printf("  -v, --version        Print the current version number\n");
  printf("  -t, --test           Test flags accepting arguments\n\n");
}

/**
 * Print help information for link command
 * command-line flags and accepted arguments
 */
void print_link_usage(char **argv) {
  printf("Usage: %s link <path> [options]\n\n", argv[0]);
  printf("Link local files or directories\n\n");
  printf("Linked files and folders are mapped to their system location based\n"
         "on the project directory structure with files in the root of the\n"
         "project mapping to the root of the system.\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n\n");
}

/**
 * Print help information for unlink command
 * command-line flags and accepted arguments
 */
void print_unlink_usage(char **argv) {
  printf("Usage: %s unlink <path> [options]\n\n", argv[0]);
  printf("Unlink local files or directories\n\n");
  printf("Linked files and folders are unmapped from their system location\n"
         "based on the project directory structure with files in the root of\n"
         "the project mapping to the root of the system.\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n\n");
}

/**
 * Print help information for list command
 * command-line flags and accepted arguments
 */
void print_list_usage(char **argv) {
  printf("Usage: %s list [options]\n\n", argv[0]);
  printf("List all tracked dotfiles\n\n");
  printf("All files discovered from the project root are listed with any\n"
         "linked files and their system location highlighted in green.\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n");
  printf("  -l, --links          Filter for files actually linked\n\n");
}

/**
 * Read file stats for a file or an expected link which
 * often won't exist but we want to handle nicely
 */
int get_file_stats(char *filename, struct stat *sb) {
  if (stat(filename, sb) == -1) {
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
      fprintf(stderr, "Unsupported file mode `%d'\n", mode);
      exit(EXIT_FAILURE);
  }
  return 0;
}

/**
 * Make the system path for some local path where a first version
 * doesn't do special mapping. Allocates memory for the returned
 * pointer which needs to be freed by the caller.
 */
char *make_link_path(const char *fpath) {
  // We're only supporting calling from the
  // project root directory for now
  char prefix[PATH_MAX + 1];
  realpath(".", prefix);
  char fabspath[PATH_MAX + 1];
  realpath(fpath, fabspath);
  char *subloc = strstr(fabspath, prefix);
  // Check explicit in case 0 location
  if (subloc == NULL) {
    fprintf(stderr, "File outside project `%s'\n", fpath);
    exit(EXIT_FAILURE);
  }
  // Using strlen(prefix) factoring in terminator
  // because the allocated length might be more
  int linklen = strlen(fabspath) - strlen(prefix);
  // Come back to why we needed to add one
  // I think it's because of NULL terminators
  char *lpath = (char *)malloc((linklen + 1) * sizeof(char));
  return strcpy(lpath, &fabspath[strlen(prefix)]);
}

/**
 * Handle a directory entry by filtering ignored
 * directories and logging the rest to stdout
 */
int treat_any_entry(const char *fpath, const struct stat *sb, int tflag) {
  // Hardcoding hidden git, stuff, and current directory
  // but should move to persisted file input later
  char *pattern = "!(./.git*|./.stuff*|.)";
  int flags = FNM_EXTMATCH;
  if (fnmatch(pattern, fpath, flags) == 0) {
    struct stat fsb, lsb;
    int errfile = get_file_stats((char *)fpath, &fsb);
    if (errfile) {
      fprintf(stderr, "Non-existent file `%s'\n", fpath);
      exit(EXIT_FAILURE);
    }
    char *lpath = make_link_path(fpath);
    int errlink = get_file_stats((char *)lpath, &lsb);
    char *spathnorm = errlink ? "x" : lpath;
    // Using fstat for both files and links to see if
    // the actual file stats are the same for both
    if (!errlink && fsb.st_ino == lsb.st_ino) {
      printf(GREEN("%s -> %s\n"), fpath, spathnorm);
    } else {
      printf("%s -> %s\n", fpath, spathnorm);
    }
    free(lpath);
  }
  return 0;
}

/**
 * Handle a directory entry by filtering paths which
 * aren't linked and logging the rest to stdout
 */
int treat_link_entry(const char *fpath, const struct stat *sb, int tflag) {
  struct stat fsb, lsb;
  int errfile = get_file_stats((char *)fpath, &fsb);
  if (errfile) {
    fprintf(stderr, "Non-existent file `%s'\n", fpath);
    exit(EXIT_FAILURE);
  }
  char *lpath = make_link_path(fpath);
  int errlink = get_file_stats((char *)lpath, &lsb);
  // Using fstat for both files and links to see if
  // the actual file stats are the same for both
  if (!errlink && fsb.st_ino == lsb.st_ino) {
    printf(GREEN("%s -> %s\n"), fpath, lpath);
  }
  free(lpath);
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
 * Tracks a link meaning creating the link and
 * persisting the mapping if the local file exists
 */
void track_link(char *fpath) {
  // Check the file actually exists
  struct stat fsb;
  int errfile = get_file_stats(fpath, &fsb);
  if (errfile) {
    fprintf(stderr, "Non-existent path `%s'\n", fpath);
    exit(EXIT_FAILURE);
  }
  char *lpath = make_link_path(fpath);
  char fabspath[PATH_MAX + 1];
  realpath(fpath, fabspath);
  // Specify the full path because locations are relative
  // to directory of the link. This implicitly throws when
  // trying to relink a file that's already linked.
  int errlink = symlink(fabspath, lpath);
  if (errlink) {
    perror("Issue creating link");
    fprintf(stderr, "Couldn't link file `%s'\n", fpath);
    exit(EXIT_FAILURE);
  }
  free(lpath);
}

/**
 * Handle LINK command
 */
void treat_link(int argc, char **argv, hidden_opts_t *hopts) {
  link_opts_t opts = {0};
  int subind = 0;
  if (set_link_options(argc, argv, &opts, &subind) != 0) {
    fprintf(stderr, "Failure setting link options\n");
    exit(EXIT_FAILURE);
  }
  if (hopts->dflag) {
    print_link_options(argc, argv, &opts);
  }
  // Current should be LINK and next should be local path
  // so if we don't have an argument just show the help
  if (++subind >= argc) {
    print_link_usage(argv);
    exit(EXIT_SUCCESS);
  }
  // The next argument is invalid since
  // we only accept a single argument
  if (++subind < argc) {
    fprintf(stderr, "Invalid link non-option `%s'\n", argv[subind]);
    exit(EXIT_SUCCESS);
  }
  // Reset for future use
  subind--;
  // We give priority to certain options
  // and stop executing depending
  if (opts.hflag) {
    print_link_usage(argv);
    exit(EXIT_SUCCESS);
  }
  // Actually add the link
  char *fpath = argv[subind];
  track_link(fpath);
}

/**
 * Unlinks a link, deletes a file, or removes
 * a directory depending on the given path
 */
void attempt_unlink(char *fpath, unlink_opts_t *opts) {
  // Check the file actually exists
  // Only unlink if we have a preimage
  struct stat fsb;
  int errfile = get_file_stats(fpath, &fsb);
  if (errfile) {
    fprintf(stderr, "Non-existent file path `%s'\n", fpath);
    exit(EXIT_FAILURE);
  }
  char *lpath = make_link_path(fpath);
  // Check if the link actually exists
  struct stat lsb;
  int errlink = get_file_stats(lpath, &lsb);
  if (errlink) {
    fprintf(stderr, "Non-existent link path `%s'\n", lpath);
    exit(EXIT_FAILURE);
  }
  // Specify the full path
  remove(lpath);
  free(lpath);
  // Still need to unmap persisted file entries but
  // we might decide to remove mapping altogether
}

/**
 * Handle UNLINK command
 */
void treat_unlink(int argc, char **argv, hidden_opts_t *hopts) {
  unlink_opts_t opts = {0};
  int subind = 0;
  if (set_unlink_options(argc, argv, &opts, &subind) != 0) {
    fprintf(stderr, "Failure setting unlink options\n");
    exit(EXIT_FAILURE);
  }
  if (hopts->dflag) {
    print_unlink_options(argc, argv, &opts);
  }
  // Current should be UNLINK and next should be local path
  // so if we don't have an argument just show the help
  if (++subind >= argc) {
    print_unlink_usage(argv);
    exit(EXIT_SUCCESS);
  }
  // The next argument is invalid since
  // we only accept a single argument
  if (++subind < argc) {
    fprintf(stderr, "Invalid unlink non-option `%s'\n", argv[subind]);
    exit(EXIT_SUCCESS);
  }
  // Reset for future use
  subind--;
  // We give priority to certain options
  // and stop executing depending
  if (opts.hflag) {
    print_unlink_usage(argv);
    exit(EXIT_SUCCESS);
  }
  // Actually remove the link
  char *fpath = argv[subind];
  attempt_unlink(fpath, &opts);
}

/**
 * Handle LIST command
 */
void treat_list(int argc, char **argv, hidden_opts_t *hopts) {
  list_opts_t opts = {0};
  int subind = 0;
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
  int concurrency_max = 20;
  if (opts.lflag) {
    if (ftw(CURRENT_DIRECTORY, treat_link_entry, concurrency_max) == -1) {
      fprintf(stderr, "Error walking directory\n");
      exit(EXIT_FAILURE);
    }
  } else {
    if (ftw(CURRENT_DIRECTORY, treat_any_entry, concurrency_max) == -1) {
      fprintf(stderr, "Error walking directory\n");
      exit(EXIT_FAILURE);
    }
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
    case LINK:
      treat_link(argc, argv, hopts);
      break;
    case LIST:
      treat_list(argc, argv, hopts);
      break;
    case UNLINK:
      treat_unlink(argc, argv, hopts);
      break;
    default:
      fprintf(stderr, "Unreachable treat_command\n");
      exit(EXIT_FAILURE);
  }
}
