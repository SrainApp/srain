/* Copyright (C) 2016-2019 Shengyu Zhang <i@silverrainz.me>
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

#ifndef __CHAT_COMMAND_H
#define __CHAT_COMMAND_H

#include "core/core.h"
#include "ret.h"

SrnRet on_command_reload(SrnCommand *cmd, void *user_data);
SrnRet on_command_server(SrnCommand *cmd, void *user_data);
SrnRet on_command_connect(SrnCommand *cmd, void *user_data);
SrnRet on_command_ignore(SrnCommand *cmd, void *user_data);
SrnRet on_command_unignore(SrnCommand *cmd, void *user_data);
SrnRet on_command_filter(SrnCommand *cmd, void *user_data);
SrnRet on_command_unfilter(SrnCommand *cmd, void *user_data);
SrnRet on_command_query(SrnCommand *cmd, void *user_data);
SrnRet on_command_unquery(SrnCommand *cmd, void *user_data);
SrnRet on_command_join(SrnCommand *cmd, void *user_data);
SrnRet on_command_part(SrnCommand *cmd, void *user_data);
SrnRet on_command_quit(SrnCommand *cmd, void *user_data);
SrnRet on_command_topic(SrnCommand *cmd, void *user_data);
SrnRet on_command_msg(SrnCommand *cmd, void *user_data);
SrnRet on_command_me(SrnCommand *cmd, void *user_data);
SrnRet on_command_nick(SrnCommand *cmd, void *user_data);
SrnRet on_command_whois(SrnCommand *cmd, void *user_data);
SrnRet on_command_invite(SrnCommand *cmd, void *user_data);
SrnRet on_command_kick(SrnCommand *cmd, void *user_data);
SrnRet on_command_mode(SrnCommand *cmd, void *user_data);
SrnRet on_command_ctcp(SrnCommand *cmd, void *user_data);
SrnRet on_command_away(SrnCommand *cmd, void *user_data);
SrnRet on_command_pattern(SrnCommand *cmd, void *user_data);
SrnRet on_command_render(SrnCommand *cmd, void *user_data);
SrnRet on_command_unrender(SrnCommand *cmd, void *user_data);
SrnRet on_command_quote(SrnCommand *cmd, void *user_data);
SrnRet on_command_clear(SrnCommand *cmd, void *user_data);

static SrnCommandBinding cmd_bindings[] = {
    {
        .name = "/reload",
        .argc = 0,
        .cb = on_command_reload,
    },
    {
        .name = "/server",
        .subcmd = {"connect", "disconnect", "list", NULL},
        .argc = 1, // <name>
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_server,
    },
    {
        .name = "/connect",
        .argc = 2, // <addr> [nick]
        .opt = {
            { .key = "-pwd",            .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-user",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-real",           .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-encode",         .val = SRN_COMMAND_OPT_NO_DEFAULT },
            { .key = "-tls",            .val = SRN_COMMAND_OPT_NO_VAL },
            { .key = "-tls-noverify",   .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_connect,
    },
    {
        .name = "/ignore",
        .argc = 1, // <nick>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .cb = on_command_ignore,
    },
    {
        .name = "/unignore",
        .argc = 1, // <nick>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .cb = on_command_unignore,
    },
    {
        .name = "/filter",
        .argc = 1, // <pattern>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .cb = on_command_filter,
    },
    {
        .name = "/unfilter",
        .argc = 1, // <pattern>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .cb = on_command_unfilter,
    },
    {
        .name = "/query",
        .alias = { "/q", NULL},
        .argc = 1, // <nick>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_query,
    },
    {
        .name = "/unquery",
        .argc = 1, // [nick]
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_unquery,
    },
    {
        .name = "/join",
        .alias = { "/j", NULL},
        .argc = 1, // <channel>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_join,
    },
    {
        .name = "/part",
        .alias = { "/leave", NULL},
        .argc = 2, // [channel] [reason]
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_part,
    },
    {
        .name = "/quit",
        .argc = 1, // [reason]
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_quit,
    },
    {
        .name = "/topic",
        .argc = 1, // [topic]
        .opt = {
            {.key = "-rm", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_topic,
    },
    {
        .name = "/msg",
        .alias = { "/m", NULL},
        .argc = 2, // <target> <message>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_msg,
    },
    {
        .name = "/me",
        .argc = 1, // <message>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_me,
    },

    {
        .name = "/nick",
        .argc = 1, // <new_nick>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_nick,
    },
    {
        .name = "/whois",
        .argc = 1, // <nick>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_whois,
    },
    {
        .name = "/invite",
        .argc = 2, // <nick> <channel>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_invite,
    },
    {
        .name = "/kick",
        .argc = 3, // <nick> <channel> <reason>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_kick,
    },
    {
        .name = "/mode",
        .argc = 2, // <target> <mode>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_mode,
    },
    {
        .name = "/ctcp",
        .argc = 3, // <nick> <command> <msg>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_ctcp,
    },
    {
        .name = "/away",
        .argc = 1, // [msg]
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_away,
    },
    {
        .name = "/pattern",
        .subcmd = {"add", "rm", "list", NULL},
        .argc = 2, // <name> [pattern]
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .flags = SRN_COMMAND_FLAG_OMIT_ARG,
        .cb = on_command_pattern,
    },
    {
        .name = "/render",
        .argc = 2, // <nick> <pattern>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .cb = on_command_render,
    },
    {
        .name = "/unrender",
        .argc = 2, // <nick> <pattern>
        .opt = {
            {.key = "-cur", .val = SRN_COMMAND_OPT_NO_VAL },
            SRN_COMMAND_EMPTY_OPT,
        },
        .cb = on_command_unrender,
    },
    {
        .name = "/quote",
        .argc = 1, // <command string>
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_quote,
    },
    {
        .name = "/clear",
        .argc = 0,
        .opt = { SRN_COMMAND_EMPTY_OPT },
        .cb = on_command_clear,
    },
    SRN_COMMAND_EMPTY,
};


#endif /* __CHAT_COMMAND_H */
