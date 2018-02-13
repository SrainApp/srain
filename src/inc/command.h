/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __COMMAND_H
#define __COMMAND_H

#include "srain.h"
#include "ret.h"

#define COMMAND_MAX_OPTS        20
#define COMMAND_MAX_ARGS        20
#define COMMAND_MAX_SUBCMD      10

#define COMMAND_OPT_PREFIX      '-'
#define COMMAND_OPT_NO_VAL      NULL
#define COMMAND_OPT_NO_DEFAULT  (const char *) 1 // FIXME: a better way?

#define COMMAND_FLAG_OMIT_ARG   1 << 1

#define COMMAND_EMPTY_OPT { .key = NULL, .val = COMMAND_OPT_NO_VAL }
#define COMMAND_EMPTY {         \
    .name = NULL,               \
    .subcmd = {NULL},           \
    .argc = 0,                  \
    .opt = {COMMAND_EMPTY_OPT}, \
    .flag = 0,                  \
    .cb = NULL,                 \
    }

typedef int CommandFlag;
typedef struct _Command Command;
typedef struct _CommandBind CommandBind;
typedef struct _CommandOption CommandOption;
typedef struct _CommandContext CommandContext;
typedef SrnRet (CommandCallback) (Command *cmd, void *user_data);

struct _CommandOption {
    const char *key;
    const char *val;
};

struct _CommandBind {
    const char *name;
    const char *subcmd[COMMAND_MAX_SUBCMD];
    const int argc;
    const CommandOption opt[COMMAND_MAX_OPTS];
    const CommandFlag flag;
    CommandCallback *cb;
};

struct _CommandContext {
    CommandBind *binds;
};

SrnRet command_proc(CommandContext *ctx, const char *rawcmd, void *user_data);
const char *command_get_subcmd(Command *cmd);
const char *command_get_arg(Command *cmd, unsigned index);
bool command_get_opt(Command *cmd, const char *opt_key, const char **opt_val);

void command_test();
void get_quote_arg_test();

#endif /* __COMMAND_H */
