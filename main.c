#include "command.h"
#include "options/hidden.h"
#include <getopt.h>
#include <stdlib.h>

static char *DEFAULT_ROOT = "/";
hidden_opts_t ghidden_opts = {0};

// Entry point for stuff
int main(int argc, char **argv) {
  ghidden_opts.rvalue = DEFAULT_ROOT;
  int subind = 0;
  set_hidden_options(argc, argv, &ghidden_opts, &subind);
  // Simpler string default
  char *command = "";
  if (argc > subind) {
    command = argv[subind];
  }
  treat_command(command, argc, argv, &ghidden_opts);
  return EXIT_SUCCESS;
}
