#include "command.h"
#include "options/hidden.h"
#include <getopt.h>

// Entry point for stuff
int main(int argc, char **argv) {
  hidden_opts_t opts = {0};
  int subind = 0;
  set_hidden_options(argc, argv, &opts, &subind);
  // Simpler string default
  char *command = "";
  if (argc > subind) {
    command = argv[subind];
  }
  treat_command(command, argc, argv, &opts);
  return 0;
}
