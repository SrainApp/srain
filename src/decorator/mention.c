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

#include <stdio.h>
#include <glib.h>

#include "core/core.h"

#include "decorator.h"

#include "srain.h"
#include "log.h"

static char* mention(Message *msg, int index, const char *frag);

Decorator mention_decroator = {
    .name = "mention",
    .func = mention,
};

static char* mention(Message *msg, int index, const char *frag){
    char *nick = NULL;
    char pattern[128];
    GError *err;
    GRegex *regex = NULL;
    GMatchInfo *match_info = NULL;

    if (msg->mentioned){
        goto FIN;
    }

    /* Genertate pattern */
    nick = g_regex_escape_string(msg->chat->srv->user->nick, -1);
    snprintf(pattern, sizeof(pattern), "\\b%s\\b", nick);

    DBG_FR("Generated pattern: %s", pattern);

    err = NULL;
    regex = g_regex_new(pattern, G_REGEX_CASELESS, 0, &err);
    if (!regex){
        ERR_FR("g_regex_new() failed: %s", err->message);
        goto FIN;
    }

    if (g_regex_match(regex, frag, 0, &match_info)){
        LOG_FR("Mentioned, nick: %s, content: %s", nick, frag);
        msg->mentioned = TRUE;
    }

FIN:
    if (match_info){
        g_match_info_free(match_info);
    }
    if (regex){
        g_regex_unref(regex);
    }
    if (nick){
        g_free(nick);
    }

    return NULL;
}
