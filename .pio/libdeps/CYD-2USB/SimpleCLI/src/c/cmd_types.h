/*
   Copyright (c) 2019 Stefan Kremser
   This software is licensed under the MIT License. See the license file for details.
   Source: github.com/spacehuhn/SimpleCLI
 */

#ifndef cmd_types_h
#define cmd_types_h

#include "c/arg_types.h" // arg

#define CMD_DEFAULT 0
#define CMD_BOUNDLESS 1
#define CMD_SINGLE 2

#define CMD_NAME_UNEQUALS 0
#define CMD_NAME_EQUALS 1

#define CMD_CASE_INSENSETIVE 0
#define CMD_CASE_SENSETIVE 1

#define CMD_SIMPLE 0
#define CMD_COMPOSITE 1

typedef struct cmd {
    const char *name;
    unsigned int mode : 2;
    struct arg *arg_list;
    unsigned int case_sensetive : 1;
    unsigned int (*callback)(struct cmd *c);
    const char *description;
    unsigned int composite : 1;
    struct cmd *cmd_list;
    struct cmd *next;
} cmd;

#endif /* ifndef cmd_types_h */