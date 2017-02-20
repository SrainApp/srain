/**
 * @file sirc_event.c
 * @brief IRC event handler
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-01-28
 *
 */

#define __LOG_ON

#include <string.h>

#include "sirc.h"
#include "sirc_numeric.h"

#include "log.h"

static void sirc_ctcp_event_hdr(SircSession *sirc, SircMessage *imsg);

void sirc_event_hdr(SircSession *sirc, SircMessage *imsg){
    int num;
    const char *origin;
    SircEvents *events;

    num = atoi(imsg->cmd);
    origin = imsg->nick ? imsg->nick : imsg->prefix;
    events = sirc_get_events(sirc);

    if (num != 0) {
        /* Numeric command */
        switch (num){
            case SIRC_RFC_RPL_WELCOME:
                events->welcome(sirc, num, origin, imsg->params, imsg->nparam, imsg->msg);
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
             switch (imsg->nparam) {
                 case 0:
                     /* User message */
                     events->privmsg(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
                     break;
                 case 1:
                     /* Channel message */
                     events->channel(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
                     break;
                 default:
                     ERR_FR("PRIVMSG has extra parameters");
                     break;
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
             switch (imsg->nparam) {
                 case 1:
                     /* User mode changed */
                     events->mode(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
                     break;
                 case 2:
                     /* Channel mode changed */
                     events->umode(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
                     break;
                 default:
                     ERR_FR("MODE has extra parameters");
                     break;
             }
         }
         else if (strcmp(event, "TOPIC") == 0){
             events->topic(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "KICK") == 0){
             events->kick(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "NOTICE") == 0){
             events->notice(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "INVITE") == 0){
             events->invite(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
     }
}

static void sirc_ctcp_event_hdr(SircSession *sirc, SircMessage *imsg) {
    int len;
    char *ctcp_msg;
    const char *origin;
    SircEvents *events;

    ctcp_msg = g_strdup(imsg->msg);
    len = strlen(ctcp_msg);
    origin = imsg->nick ? imsg->nick : imsg->prefix;
    events = sirc_get_events(sirc);

    ctcp_msg++; // Skip first 0x01
    ctcp_msg[len-1] = '\0'; // Remove the trailing 0x01

    if (strncasecmp(ctcp_msg, "ACTION ", sizeof("ACTION ") - 1) == 0){
        ctcp_msg += sizeof("ACTION ") - 1;
        events->ctcp_action(sirc, "CTCP_ACTION", origin, imsg->params, imsg->nparam, ctcp_msg);
    }
    else if (strncasecmp(ctcp_msg, "DCC ", sizeof("DCC ") - 1) == 0){
        // TODO: impl DCC
    } else {
        // TODO: Any other CTCP event?
    }

    g_free(ctcp_msg);
}
