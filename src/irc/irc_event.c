/**
 * @file irc_event.c
 * @brief IRC event handler
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 
 * @date 2017-01-28
 *
 */

#define __LOG_ON

#include "irc_event.h"

#include "log.h"

void irc_event_init(){
    /*
    */
}

void irc_event_hdr(SircSession *sirc, IRCMsg *imsg){
    int num = atoi(imsg->command);
    char *origin = imsg->nick ? imsg->nick : imsg->prefix;

    if (num != 0) {
        /* Numeric command */
        switch (num){
            case RPL_WELCOME:
                sirc->events.welcome(sirc->ctx, num, origin, imsg->param, imsg->nparam);
            default:
                sirc->events.numeric(sirc->ctx, num, origin, imsg->param, imsg->nparam);
        }
     } else {
        /* Named command */
         const char* event = imsg->command;
         if (strcmp(event, "PRIVMSG") == 0){
             sirc->events.privmsg(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "JOIN") == 0){
             sirc->events.join(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "PART") == 0){
             sirc->events.part(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "QUIT") == 0){
             sirc->events.quit(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "NICK") == 0){
             sirc->events.nick(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "MODE") == 0){
             sirc->events.mode(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "UMODE") == 0){
             sirc->events.umode(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "TOPIC") == 0){
             sirc->events.topic(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "KICK") == 0){
             sirc->events.kick(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "CHANNEL") == 0){
             sirc->events.channel(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "NOTICE") == 0){
             sirc->events.notice(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "CHANNEL_NOTICE") == 0){
             sirc->events.channel_notice(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "INVITE") == 0){
             sirc->events.invite(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "CTCP_ACTION") == 0){
             sirc->events.ctcp_action(sirc->ctx, event, origin, imsg->param, imsg->nparam);
         }
     }
}
