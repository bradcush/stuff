// Enable GNU extension required for FNM_EXTMATCH
// We should replace this to not rely on GNU systems
#define _GNU_SOURCE

#include "command.h"
#include "options/hidden.h"
#include "options/link.h"
#include "options/list.h"
#include "options/none.h"
#include "options/unlink.h"
#include <errno.h>
#include <fnmatch.h>
#include <ftw.h>
#include <limits.h>
#include <pwd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// Supress unused compiler warnings with a
// compiler specific variable attribute
#define UNUSED __attribute__((unused))

// Defining colored output
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define GREEN(str) ANSI_COLOR_GREEN str ANSI_COLOR_RESET

// Setting some options globally for now so we can
// access them in places we can't easily get to like
// inside functions passed as pointers to ftw
static list_opts_t glist_opts = {0};

static const char *ROOT_DIRECTORY = "/";
static const char *CURRENT_DIRECTORY = ".";
static const char *VERSION = "0.0.1";

/**
 * Map a command to a better suited enum
 */
command_t map_command(char *command) {
  const struct {
    command_t val;
    const char *str;
  } map[] = {{NONE, ""}, {LINK, "link"}, {LIST, "list"}, {UNLINK, "unlink"}};
  size_t length = sizeof(map) / sizeof(map[0]);
  for (size_t i = 0; i < length; i++) {
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
  printf(
      "stuff is a tool intended to make it simple to manage dotfiles\n"
      "using a repository with version control. There is a list of\n"
      "commands and options below that can be called with the additional\n"
      "`--help' flag for more information.\n\n"
  );
  printf("Commands:\n");
  printf("  link                 Link local files or directories\n");
  printf("  list                 List all of the tracked dotfiles\n");
  printf("  unlink               Unlink local files or directories\n\n");
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n");
  printf("  -v, --version        Print the current version number\n");
  printf("  -r, --root           Specify a path for another link location\n\n");
}

/**
 * Print help information for link command
 * command-line flags and accepted arguments
 */
void print_link_usage(char **argv) {
  printf("Usage: %s link <path> [options]\n\n", argv[0]);
  printf("Link local files or directories\n\n");
  printf(
      "Linked files and folders are mapped to their system location based\n"
      "on the project directory structure with files in the root of the\n"
      "project mapping to the root of the system.\n\n"
  );
  printf("Options:\n");
  printf("  -h, --force          Link even if a link exists\n");
  printf("  -h, --help           Print this help and exit\n\n");
}

/**
 * Print help information for unlink command
 * command-line flags and accepted arguments
 */
void print_unlink_usage(char **argv) {
  printf("Usage: %s unlink <path> [options]\n\n", argv[0]);
  printf("Unlink local files or directories\n\n");
  printf(
      "Linked files and folders are unmapped from their system location\n"
      "based on the project directory structure with files in the root of\n"
      "the project mapping to the root of the system.\n\n"
  );
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n\n");
}

/**
 * Print help information for list command
 * command-line flags and accepted arguments
 */
void print_list_usage(char **argv) {
  printf("Usage: %s list [options]\n\n", argv[0]);
  printf("List all of the tracked dotfiles\n\n");
  printf(
      "All files discovered from the project root are listed using\n"
      "their system location with links highlighted in green.\n\n"
  );
  printf("Options:\n");
  printf("  -h, --help           Print this help and exit\n");
  printf("  -l, --linked         Filter for files actually linked\n");
  printf("  -o, --owner          List the owner with the linked file\n\n");
}

/**
 * Read file stats for a file or an expected link which
 * often won't exist but we want to handle nicely
 */
int get_file_stats(const char *filename, struct stat *sb) {
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
 * pointer which needs to be freed by the caller. This also
 * handles a custom root defined in the hidden options.
 */
char *make_link_path(const char *fpath) {
  // We're only supporting calling from the
  // project root directory for now
  char prefix[PATH_MAX + 1];
  realpath(CURRENT_DIRECTORY, prefix);
  char fabspath[PATH_MAX + 1];
  realpath(fpath, fabspath);
  char *pos = strstr(fabspath, prefix);
  // Substring doesn't match
  if (pos == NULL) {
    fprintf(stderr, "File outside project `%s'\n", fpath);
    exit(EXIT_FAILURE);
  }
  char next_prefix[PATH_MAX + 1];
  realpath(ghidden_opts.rvalue, next_prefix);
  char *suffix = &fabspath[strlen(prefix) + 1];
  // Adding 1 makes enough room to add "/" if needed
  int lpathlen = strlen(next_prefix) + 1 + strlen(suffix);
  char *lpath = (char *)malloc((lpathlen + 1) * sizeof(char));
  strcpy(lpath, next_prefix);
  // Just next_prefix = "/" ends with a slash so
  // we need to add it for all the other cases
  if (strcmp(next_prefix, ROOT_DIRECTORY)) {
    strcat(lpath, ROOT_DIRECTORY);
  }
  return strcat(lpath, suffix);
}

/**
 * Given a link path and name buffer, fills the
 * name buffer with the user who owns the link
 */
char *get_link_owner(char *lpath) {
  struct stat lsb;
  // Gives the stats for the link instead
  // of the file that the link links to
  lstat(lpath, &lsb);
  struct passwd *pwd;
  pwd = getpwuid(lsb.st_uid);
  char *username = pwd->pw_name;
  char *owner = (char *)malloc((strlen(username) + 1) * sizeof(char));
  return strcpy(owner, username);
}

/**
 * Handle a file or directory entry based on
 * global list options and log to stdout
 */
int treat_entry(
    const char *fpath, UNUSED const struct stat *sb, UNUSED int tflag
) {
  // Hardcoding hidden git, stuff, and current directory
  // but should move to persisted file input later
  char *pattern = "!(./.git*|./.stuff*|.)";
  int flags = FNM_EXTMATCH;
  if (fnmatch(pattern, fpath, flags) == 0) {
    struct stat fsb, lsb;
    int errfile = get_file_stats(fpath, &fsb);
    if (errfile) {
      fprintf(stderr, "Non-existent file `%s'\n", fpath);
      exit(EXIT_FAILURE);
    }
    char *lpath = make_link_path(fpath);
    int errlink = get_file_stats(lpath, &lsb);
    // Using fstat for both files and links to see if
    // the actual file stats are the same for both. This
    // doesn't distinguish between a link and a file.
    if (!errlink && fsb.st_ino == lsb.st_ino) {
      if (glist_opts.oflag) {
        char *owner = get_link_owner(lpath);
        printf(GREEN("%s %s\n"), owner, lpath);
      } else {
        printf(GREEN("%s") "\n", lpath);
      }
    } else if (!glist_opts.lflag) {
      // Don't care about unlinked owners
      printf("%s\n", fpath);
    }
    free(lpath);
  }
  return 0;
}

/**
 * Handle NONE command
 */
void treat_none(int argc, char **argv) {
  none_opts_t opts = {0, 0};
  int subind = 0;
  if (set_none_options(argc, argv, &opts, &subind) != 0) {
    fprintf(stderr, "Failure setting options\n");
    exit(EXIT_FAILURE);
  }
  if (ghidden_opts.dflag) {
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
    printf("stuff version %s\n", VERSION);
  }
  // Best to call usage explicitly
  // when no arguments given
  if (argc == 1) {
    print_none_usage(argv);
  }
}

/**
 * Unlinks a link, deletes a file, or removes
 * a directory depending on the given path
 */
void attempt_unlink(char *fpath) {
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
  int errunlink = remove(lpath);
  if (errunlink) {
    perror("Issue unlinking path");
    fprintf(stderr, "Couldn't unlink path `%s'\n", lpath);
    exit(EXIT_FAILURE);
  }
  free(lpath);
}

/**
 * Tracks a link, meaning creates it
 */
void add_link(char *fpath, char *lpath, link_opts_t *opts) {
  // Check the file actually exists
  struct stat fsb;
  int errfile = get_file_stats(fpath, &fsb);
  if (errfile) {
    fprintf(stderr, "Non-existent path `%s'\n", fpath);
    exit(EXIT_FAILURE);
  }
  char fabspath[PATH_MAX + 1];
  realpath(fpath, fabspath);
  // Specify the full path because locations are relative
  // to directory of the link. This implicitly throws when
  // trying to relink a file that's already linked.
  int errlink = symlink(fabspath, lpath);
  if (errlink) {
    if (errno == EEXIST && opts->fflag) {
      // Useful when downgrading permissions
      attempt_unlink(fpath);
      int nerrlink = symlink(fabspath, lpath);
      if (nerrlink) {
        perror("Issue creating forced link");
        fprintf(stderr, "Couldn't force link file `%s'\n", fpath);
        exit(EXIT_FAILURE);
      }
    } else {
      perror("Issue creating link");
      fprintf(stderr, "Couldn't link file `%s'\n", fpath);
      exit(EXIT_FAILURE);
    }
  }
}

/**
 * Handle LINK command
 */
void treat_link(int argc, char **argv) {
  link_opts_t opts = {0};
  int subind = 0;
  if (set_link_options(argc, argv, &opts, &subind) != 0) {
    fprintf(stderr, "Failure setting link options\n");
    exit(EXIT_FAILURE);
  }
  if (ghidden_opts.dflag) {
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
    exit(EXIT_FAILURE);
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
  char *lpath = make_link_path(fpath);
  add_link(fpath, lpath, &opts);
  printf(GREEN("%s") "\n", lpath);
  free(lpath);
}

/**
 * Handle UNLINK command
 */
void treat_unlink(int argc, char **argv) {
  unlink_opts_t opts = {0};
  int subind = 0;
  if (set_unlink_options(argc, argv, &opts, &subind) != 0) {
    fprintf(stderr, "Failure setting unlink options\n");
    exit(EXIT_FAILURE);
  }
  if (ghidden_opts.dflag) {
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
    exit(EXIT_FAILURE);
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
  attempt_unlink(fpath);
  printf("%s\n", fpath);
}

/**
 * Handle LIST command
 */
void treat_list(int argc, char **argv) {
  int subind = 0;
  if (set_list_options(argc, argv, &glist_opts, &subind) != 0) {
    fprintf(stderr, "Failure setting list options\n");
    exit(EXIT_FAILURE);
  }
  if (ghidden_opts.dflag) {
    print_list_options(argc, argv, &glist_opts);
  }
  // Current should be LIST so next
  // is invalid if within limit
  if (++subind < argc) {
    fprintf(stderr, "Invalid list non-option `%s'\n", argv[subind]);
    exit(EXIT_FAILURE);
  }
  // We give priority to certain options
  // and stop executing depending
  if (glist_opts.hflag) {
    print_list_usage(argv);
    exit(EXIT_SUCCESS);
  }
  // Concurrently handle 20 entries at a time
  if (ftw(CURRENT_DIRECTORY, treat_entry, 20) == -1) {
    fprintf(stderr, "Error walking directory\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * Handle functionality specific to a command or lack thereof
 */
void treat_command(char *command, int argc, char **argv) {
  switch (map_command(command)) {
    case NONE:
      treat_none(argc, argv);
      break;
    case LINK:
      treat_link(argc, argv);
      break;
    case LIST:
      treat_list(argc, argv);
      break;
    case UNLINK:
      treat_unlink(argc, argv);
      break;
    default:
      fprintf(stderr, "Unreachable treat_command\n");
      exit(EXIT_FAILURE);
  }
}
