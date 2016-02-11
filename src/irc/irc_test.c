#include <stdio.h>
#include <string.h>
#include "irc.h"
#include "log.h"

int main(){
    irc_t irc;
    irc_msg_t ircmsg;

    irc_connect(&irc, "irc.freenode.net", "6666");
    irc_login(&irc, "srainbot");
    irc_join_chan(&irc, "#lasttest");
    for (;;){
        memset(&ircmsg, 0, sizeof(irc_msg_t));
        if (irc_recv(&irc, &ircmsg) == IRCMSG_UNKNOWN){
            // ERR_FR("IRCMSG_UNKNOWN");
            continue;
        }
        LOG_FR("{%s}", ircmsg.message);
    }
    return 0;
}
