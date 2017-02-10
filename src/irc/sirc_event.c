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

#include "sirc_numeric.h"
#include "sirc_event.h"

#include "log.h"

void sirc_event_hdr(SircSession *sirc, SircMessage *imsg){
    int num = atoi(imsg->cmd);
    const char *origin = imsg->nick ? imsg->nick : imsg->prefix;

    if (num != 0) {
        /* Numeric command */
        switch (num){
            case RPL_WELCOME:
                sirc->events.welcome(sirc, num, origin, imsg->params, imsg->nparam, imsg->msg);
            default:
                sirc->events.numeric(sirc, num, origin, imsg->params, imsg->nparam, imsg->msg);
        }
     } else {
        /* Named command */
         const char* event = imsg->cmd;
         if (strcmp(event, "PRIVMSG") == 0){
             sirc->events.privmsg(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "JOIN") == 0){
             sirc->events.join(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "PART") == 0){
             sirc->events.part(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "QUIT") == 0){
             sirc->events.quit(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "NICK") == 0){
             sirc->events.nick(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "MODE") == 0){
             sirc->events.mode(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "UMODE") == 0){
             sirc->events.umode(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "TOPIC") == 0){
             sirc->events.topic(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "KICK") == 0){
             sirc->events.kick(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "CHANNEL") == 0){
             sirc->events.channel(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "NOTICE") == 0){
             sirc->events.notice(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "CHANNEL_NOTICE") == 0){
             sirc->events.channel_notice(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "INVITE") == 0){
             sirc->events.invite(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
         else if (strcmp(event, "CTCP_ACTION") == 0){
             sirc->events.ctcp_action(sirc, event, origin, imsg->params, imsg->nparam, imsg->msg);
         }
     }
}
