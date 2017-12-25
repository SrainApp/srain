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

/**
 * @file sirc_event_hdr.c
 * @brief IRC event handler
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2017-01-28
 *
 */

#include <stdlib.h>
#include <string.h>

#include "sirc_event_hdr.h"

#include "srain.h"
#include "log.h"

static void sirc_ctcp_event_hdr(SircSession *sirc, SircMessage *imsg);

void sirc_event_hdr(SircSession *sirc, SircMessage *imsg){
    int num;
    bool nullparam;
    const char *origin;
    const char **params;
    SircEvents *events;

    g_return_if_fail(imsg->nick || imsg->prefix);

    events = sirc_get_events(sirc);
    num = atoi(imsg->cmd);

    /* Cast to immutable string */
    origin = imsg->nick ? imsg->nick : imsg->prefix;
    params = (const char **)imsg->params;

    /* Debug output and parameters check */
    nullparam = FALSE;
    DBG_FR("sirc: %p, event: %s, origin: %s", sirc, imsg->cmd, origin);
    for (int i = 0; i < imsg->nparam; i++){
        if (i == 0) DBG_F("count: %d, params: [", imsg->nparam);
        if (i == imsg->nparam - 1) {
            DBG("%s]\n", params[i]);
        } else {
            DBG("%s ", params[i]);
        }
        if (!params[i]) nullparam = TRUE;
    }
    g_return_if_fail(!nullparam);

    if (num != 0) {
        /* Numeric command */
        switch (num){
            case SIRC_RFC_RPL_WELCOME:
                g_return_if_fail(events->welcome);
                events->welcome(sirc, num, origin, params, imsg->nparam);
                /* Do not break here */
            default:
                g_return_if_fail(events->numeric);
                events->numeric(sirc, num, origin, params, imsg->nparam);
        }
     } else {
        /* Named command */
         const char* event = imsg->cmd;

         if (strcmp(event, "PRIVMSG") == 0){
             g_return_if_fail(imsg->nparam >= 2);

             const char *target = params[0];
             const char *msg = params[1];

             int len = strlen(msg);
             /* Check for CTCP request (starts and ends with 0x01) */
             if (len >= 2 && msg[0] == '\x01' && msg[1] == '\x01') {
                 sirc_ctcp_event_hdr(sirc, imsg);
                 return;
             }

             if (sirc_is_chan(target)){
                 /* Channel message */
                g_return_if_fail(events->channel);
                 events->channel(sirc, event, origin, params, imsg->nparam);
             } else {
                 /* User message */
                 g_return_if_fail(events->privmsg);
                 events->privmsg(sirc, event, origin, params, imsg->nparam);
             }
         }
         else if (strcmp(event, "JOIN") == 0){
             g_return_if_fail(events->join);
             events->join(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "PART") == 0){
             g_return_if_fail(events->part);
             events->part(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "QUIT") == 0){
             g_return_if_fail(events->quit);
             events->quit(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "NICK") == 0){
             g_return_if_fail(events->nick);
             events->nick(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "MODE") == 0){
             g_return_if_fail(imsg->nparam >= 1);
             if (sirc_is_chan(params[0])){
                 /* Channel mode changed */
                 g_return_if_fail(events->mode);
                 events->mode(sirc, event, origin, params, imsg->nparam);
             } else {
                 /* User mode changed */
                 g_return_if_fail(events->umode);
                 events->umode(sirc, event, origin, params, imsg->nparam);
             }
         }
         else if (strcmp(event, "TOPIC") == 0){
             g_return_if_fail(events->topic);
             events->topic(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "KICK") == 0){
             g_return_if_fail(events->kick);
             events->kick(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "NOTICE") == 0){
             g_return_if_fail(imsg->nparam >= 1);
             if (sirc_is_chan(params[0])){
                 /* Channel notice changed */
                 g_return_if_fail(events->channel_notice);
                 events->channel_notice(sirc, event, origin, params, imsg->nparam);
             } else {
                 /* User notice message */
                 g_return_if_fail(events->notice);
                 events->notice(sirc, event, origin, params, imsg->nparam);
             }
         }
         else if (strcmp(event, "INVITE") == 0){
             g_return_if_fail(events->invite);
             events->invite(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "PING") == 0){
             g_return_if_fail(events->ping);
             events->ping(sirc, event, origin, params, imsg->nparam);
             /* Response "PING" message */
             // FIXME: response all params?
             sirc_cmd_pong(sirc, params[imsg->nparam - 1]);
         }
         else if (strcmp(event, "PONG") == 0){
             g_return_if_fail(events->pong);
             events->pong(sirc, event, origin, params, imsg->nparam);
         }
         else if (strcmp(event, "ERROR") == 0){
             g_return_if_fail(events->error);
             events->error(sirc, event, origin, params, imsg->nparam);
         }
         else {
             g_return_if_fail(events->unknown);
             events->unknown(sirc, event, origin, params, imsg->nparam);
         }
     }
}

static void sirc_ctcp_event_hdr(SircSession *sirc, SircMessage *imsg) {
    int len;
    char *ptr;
    char *tmp;
    char *ctcp_msg;
    const char *origin;
    const char **params;
    SircEvents *events;

    events = sirc_get_events(sirc);
    tmp = imsg->params[imsg->nparam - 1];
    ptr = ctcp_msg = g_strdup(tmp);
    len = strlen(ctcp_msg);

    /* Cast to immutable string */
    origin = imsg->nick ? imsg->nick : imsg->prefix;
    params = (const char **)imsg->params;

    ptr++; // Skip first 0x01
    ctcp_msg[len-1] = '\0'; // Remove the trailing 0x01

    if (strncmp(ptr, "ACTION ", sizeof("ACTION ") - 1) == 0){
        ptr += sizeof("ACTION ") - 1;
        imsg->params[imsg->nparam - 1] = ptr;
        events->ctcp_action(sirc, "CTCP_ACTION", origin, params, imsg->nparam);
    }
    else if (strncmp(ptr, "DCC ", sizeof("DCC ") - 1) == 0){
        // TODO: impl DCC
    } else {
        // TODO: Any other CTCP event?
    }

    imsg->params[imsg->nparam - 1] = tmp; // Recover parameter
    g_free(ctcp_msg);
}
