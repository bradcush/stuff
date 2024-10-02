#include "options.h"

// Entry point for stuff
int main(int argc, char **argv) {
  opts_t opts = {0, 0, NULL};
  char default_mode = 'A';
  opts.tvalue = &default_mode;
  int code = set_options(argc, argv, &opts);
  if (code != 0) {
    return code;
  }
  if (opts.vflag) {
    print_options(argc, argv, &opts);
  }
  return 0;
}
