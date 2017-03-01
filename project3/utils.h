#ifndef UTILS_H
#define UTILS_H

#include "types.h"

#include <ctype.h>

struct server_command{
    char *cmd;
    int argc;
    char **argv;
};

struct server_command create_command(char *mycmd, int length);

void delete_command(struct server_command scmd);

bool is_number(char *str);

int int_cmp(const void *a, const void *b);

bool safe_read(int fd, char *buffer, int length);
#endif
