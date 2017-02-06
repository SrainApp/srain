/**
 * @file irc_event.c
 * @brief IRC event handler
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 
 * @date 2017-01-28
 *
 */

#define __LOG_ON

#include "srv_event.h"
#include "irc_event.h"

#include "log.h"

static IRCEvents events;

void irc_event_init(){
    events.connect  = srv_event_connect;
    events.nick = srv_event_nick;
    events.quit = srv_event_quit;
    events.join = srv_event_join;
    events.part = srv_event_part;
    events.mode = srv_event_mode;
    events.umode = srv_event_umode;
    events.topic = srv_event_topic;
    events.kick = srv_event_kick;
    events.channel = srv_event_channel;
    events.privmsg = srv_event_privmsg;
    events.notice = srv_event_notice;
    events.channel_notice = srv_event_channel_notice;
    events.invite = srv_event_invite;
    events.ctcp_action = srv_event_ctcp_action;
    events.numeric = srv_event_numeric;
}

void irc_event_hdr(Server *srv, IRCMsg *imsg){
    int num = atoi(imsg->command);
    char *origin = imsg->nick ? imsg->nick : imsg->prefix;

    if (num != 0) {
        /* Numeric command */
        switch (num){
            case RPL_WELCOME:
                events.connect(srv, num, origin, imsg->param, imsg->nparam);
            default:
                events.numeric(srv, num, origin, imsg->param, imsg->nparam);
        }
     } else {
        /* Named command */
         const char* event = imsg->command;
         if (strcmp(event, "PRIVMSG") == 0){
             events.privmsg(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "JOIN") == 0){
             events.join(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "PART") == 0){
             events.part(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "QUIT") == 0){
             events.quit(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "NICK") == 0){
             events.nick(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "MODE") == 0){
             events.mode(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "UMODE") == 0){
             events.umode(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "TOPIC") == 0){
             events.topic(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "KICK") == 0){
             events.kick(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "CHANNEL") == 0){
             events.channel(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "NOTICE") == 0){
             events.notice(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "CHANNEL_NOTICE") == 0){
             events.channel_notice(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "INVITE") == 0){
             events.invite(srv, event, origin, imsg->param, imsg->nparam);
         }
         else if (strcmp(event, "CTCP_ACTION") == 0){
             events.ctcp_action(srv, event, origin, imsg->param, imsg->nparam);
         }
     }
}
