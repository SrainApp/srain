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

#define SRN_COMMAND_MAX_OPTS        20
#define SRN_COMMAND_MAX_ARGS        20
#define SRN_COMMAND_MAX_SUBCMD      10

#define SRN_COMMAND_OPT_PREFIX      '-'
#define SRN_COMMAND_OPT_NO_VAL      NULL
#define SRN_COMMAND_OPT_NO_DEFAULT  (const char *) 1 // FIXME: a better way?

#define SRN_COMMAND_FLAG_OMIT_ARG   1 << 1

#define SRN_COMMAND_EMPTY_OPT { .key = NULL, .val = SRN_COMMAND_OPT_NO_VAL }
#define SRN_COMMAND_EMPTY {         \
    .name = NULL,                   \
    .subcmd = {NULL},               \
    .argc = 0,                      \
    .opt = {SRN_COMMAND_EMPTY_OPT}, \
    .flag = 0,                      \
    .cb = NULL,                     \
    }

typedef int SrnCommandFlag;
typedef struct _SrnCommand SrnCommand;
typedef struct _SrnCommandBind SrnCommandBind;
typedef struct _SrnCommandOption SrnCommandOption;
typedef struct _SrnCommandContext SrnCommandContext;
typedef SrnRet (SrnCommandCallback) (SrnCommand *cmd, void *user_data);

struct _SrnCommandOption {
    const char *key;
    const char *val;
};

struct _SrnCommandBind {
    const char *name;
    const char *subcmd[SRN_COMMAND_MAX_SUBCMD];
    const int argc;
    const SrnCommandOption opt[SRN_COMMAND_MAX_OPTS];
    const SrnCommandFlag flag;
    SrnCommandCallback *cb;
};

struct _SrnCommandContext {
    SrnCommandBind *binds;
};

SrnRet srn_command_proc(SrnCommandContext *ctx, const char *rawcmd, void *user_data);
const char *srn_command_get_subcmd(SrnCommand *cmd);
const char *srn_command_get_arg(SrnCommand *cmd, unsigned index);
bool srn_command_get_opt(SrnCommand *cmd, const char *opt_key, const char **opt_val);

void command_test();
void get_quote_arg_test();

#endif /* __COMMAND_H */
