#include <stdio.h>
#include "irc.h"
#include "log.h"

int main(){
    IRC irc;
    IRCMsg ircmsg;

    irc_connect(&irc, "irc.freenode.net", "6666");
    irc_login(&irc, "srainbot");
    irc_join_chan(&irc, "#lasttest");
    for (;;){
        if (irc_recv(&irc, &ircmsg) == IRCMSG_UNKNOWN){
            ERR_FR("IRCMSG_UNKNOWN");
            return -1;
        }
        LOG_FR("{ %s }", ircmsg.message);
    }
    return 0;
}
