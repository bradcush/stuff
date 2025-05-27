#include "options/hidden.h"
#include "options/none.h"
#include <stddef.h>

#ifndef COMMAND_H
#define COMMAND_H

typedef enum command { NONE, INIT, LINK, LIST, UNLINK } command_t;

void treat_command(char *command, int argc, char **argv, hidden_opts_t *hopts);

#endif
