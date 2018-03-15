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

/**
 * @file chat_log.c
 * @brief Message filter for chat logging
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-08-21
 */


#include <stdio.h>
#include <glib.h>

#include "core/core.h"

#include "filter.h"

#include "meta.h"
#include "log.h"
#include "srain.h"
#include "log.h"
#include "i18n.h"
#include "file_helper.h"

static bool chat_log(const Message *msg, const char *content);

Filter chat_log_filter = {
    .name = "chat_log",
    .func = chat_log,
};

static bool chat_log(const Message *msg, const char *content){
    char timestr[32];
    char datestr[32];
    FILE *fp;
    char *file;
    GString *basename;

    strftime(timestr, 32, "%T", localtime(&msg->time));
    timestr[31] = '\0';
    strftime(datestr, 32, "%F", localtime(&msg->time));
    datestr[31] = '\0';

    basename = g_string_new("");

    g_string_append_printf(basename, "%s.%s.log", datestr, msg->chat->name);
    file = create_log_file(msg->chat->srv->cfg->name, basename->str);

    if (!file){
        ERR_FR("Failed to create log file");
        goto cleanup1;
    }

    fp = fopen(file, "a+");
    if (!fp){
        ERR_FR("Failed to open file '%s'", file);
        goto cleanup2;
    }

    switch (msg->type){
        case MESSAGE_SENT:
            fprintf(fp,"[%s] <%s*> %s\n", timestr, msg->user->nick, msg->content);
            break;
        case MESSAGE_RECV:
        case MESSAGE_NOTICE:
            fprintf(fp,"[%s] <%s> %s\n", timestr, msg->user->nick, msg->content);
            break;
        case MESSAGE_ACTION:
            fprintf(fp,"[%s] * %s %s\n", timestr, msg->user->nick, msg->content);
            break;
        case MESSAGE_MISC:
            fprintf(fp,"[%s] = %s\n", timestr, msg->content);
            break;
        case MESSAGE_ERROR:
            fprintf(fp,"[%s] ! %s\n", timestr, msg->content);
            break;
        case MESSAGE_UNKNOWN:
        default:
            break;
    }

    fclose(fp);
cleanup2:
    g_free(file);
cleanup1:
    g_string_free(basename, TRUE);

    return TRUE; // Always TRUE
}
