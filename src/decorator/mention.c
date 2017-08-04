/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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

#include <stdio.h>
#include <glib.h>

#include "server.h"

#include "decorator.h"

#include "srain.h"
#include "log.h"

static int mention(Message *msg, DecoratorFlag flag, void *user_data);

Decorator mention_decroator = {
    .name = "mention",
    .func = mention,
};


static int mention(Message *msg, DecoratorFlag flag, void *user_data){
    char *nick;
    char pattern[128];
    GError *err;
    GRegex *regex;
    GMatchInfo *match_info;

    /* Genertate pattern */
    nick = g_regex_escape_string(msg->chat->srv->user->nick, -1);
    snprintf(pattern, sizeof(pattern), "\\b%s\\b", nick);
    g_free(nick);

    DBG_FR("Generated pattern: %s", pattern);

    err = NULL;
    regex = g_regex_new(pattern, G_REGEX_CASELESS, 0, &err);
    if (!regex){
        ERR_FR("g_regex_new() failed: %s", err->message);
        return SRN_ERR;
    }

    if (g_regex_match(regex, msg->dcontent, 0, &match_info)){
        DBG_FR("Matchted");
        msg->mentioned = TRUE;
    }

    g_match_info_free(match_info);
    g_regex_unref(regex);

    return SRN_OK;
}
