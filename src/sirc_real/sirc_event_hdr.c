/**
 * @file sirc_event_hdr.c
 * @brief IRC event handler
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-01-28
 *
 */

// #define __DBG_ON
#define __LOG_ON

#include <string.h>

#include "sirc_event_hdr.h"

#include "srain.h"
#include "log.h"

static void sirc_ctcp_event_hdr(SircSession *sirc, SircMessage *imsg);

void sirc_event_hdr(SircSession *sirc, SircMessage *imsg){
    int num;
    const char *origin;
    SircEvents *events;

    events = sirc_get_events(sirc);

    if (imsg->type != SIRC_MSG_MESSAGE) {
        DBG_FR("Miscellaneous messages, sirc: %p, type: %d, msg: %s",
                sirc, imsg->type, imsg->msg);
    };

    /* Miscellaneous messages */
    switch (imsg->type){
        case SIRC_MSG_PING:
            sirc_cmd_pong(sirc, imsg->msg);
            events->ping(sirc, "PING");
            return;
        case SIRC_MSG_PONG:
        case SIRC_MSG_NOTICE:
        case SIRC_MSG_ERROR:
            return; // TODO:
        case SIRC_MSG_MESSAGE:
            break;
    }

    num = atoi(imsg->cmd);
    origin = imsg->nick ? imsg->nick : imsg->prefix;

    /* Debug output and parameters check */
    bool nullparam = FALSE;
    DBG_FR("sirc: %p, event: %s, origin: %s", sirc, imsg->cmd, origin);
    DBG_FR("msg: %s", imsg->msg);
    for (int i = 0; i < imsg->nparam; i++){
        if (i == 0) DBG_F("count: %d, params: [", imsg->nparam);
        if (i == imsg->nparam - 1) {
            DBG("%s]\n", imsg->params[i]);
        } else {
            DBG("%s ", imsg->params[i]);
        }
        if (!imsg->params[i]) nullparam = TRUE;
    }
    g_return_if_fail(!nullparam);

    if (num != 0) {
        /* Numeric command */
        switch (num){
            case SIRC_RFC_RPL_WELCOME:
                events->welcome(sirc, num, origin, imsg->params, imsg->nparam, imsg->msg);
                /* Do not break here */
            default:
                events->numeric(sirc, num, origin, imsg->params, imsg->nparam, imsg->msg);
        }
     } else {
        /* Named command */
         const char* event = imsg->cmd;

         if (strcmp(event, "PRIVMSG") == 0){
             /* Check for CTCP request (starts and ends with 0x01) */
             int len = strlen(imsg->msg);
             if (len >= 2 && imsg->msg[0] == '\x01' && imsg->msg[len-1] == '\x01') {
                 sirc_ctcp_event_hdr(sirc, imsg);
                 return;
             }

             g_return_if_fail(imsg->nparam >= 1);
             if (sirc_is_chan(imsg->params[0])){
                 /* Channel message */
                 events->channel(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
             } else {
                 /* User message */
                 events->privmsg(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
             }
         }
         else if (strcmp(event, "JOIN") == 0){
             events->join(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "PART") == 0){
             events->part(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "QUIT") == 0){
             events->quit(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "NICK") == 0){
             events->nick(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "MODE") == 0){
             g_return_if_fail(imsg->nparam >= 1);
             if (sirc_is_chan(imsg->params[0])){
                 /* Channel mode changed */
                 events->mode(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
             } else {
                 /* User mode changed */
                 events->umode(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
             }
         }
         else if (strcmp(event, "TOPIC") == 0){
             events->topic(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "KICK") == 0){
             events->kick(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "NOTICE") == 0){
             g_return_if_fail(imsg->nparam >= 1);
             if (sirc_is_chan(imsg->params[0])){
                 /* Channel notice changed */
                 events->channel_notice(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
             } else {
                 /* User notice message */
                 events->notice(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
             }
         }
         else if (strcmp(event, "INVITE") == 0){
             events->invite(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
     }
}

static void sirc_ctcp_event_hdr(SircSession *sirc, SircMessage *imsg) {
    int len;
    char *ptr;
    char *ctcp_msg;
    const char *origin;
    SircEvents *events;

    ctcp_msg = g_strdup(imsg->msg);
    ptr = ctcp_msg;
    len = strlen(ctcp_msg);

    origin = imsg->nick ? imsg->nick : imsg->prefix;
    events = sirc_get_events(sirc);

    ptr++; // Skip first 0x01
    ctcp_msg[len-1] = '\0'; // Remove the trailing 0x01

    if (strncmp(ptr, "ACTION ", sizeof("ACTION ") - 1) == 0){
        ptr += sizeof("ACTION ") - 1;
        events->ctcp_action(sirc, "CTCP_ACTION", origin, imsg->params, imsg->nparam, ptr);
    }
    else if (strncmp(ptr, "DCC ", sizeof("DCC ") - 1) == 0){
        // TODO: impl DCC
    } else {
        // TODO: Any other CTCP event?
    }

    g_free(ctcp_msg);
}
