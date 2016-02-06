#include <stdio.h>
#include "irc.h"
#include "log.h"

int main(){
    IRC irc;
    char cmd[32];
    char msg[2048];
    char nick[NICK_LEN];
    char chan[CHAN_LEN];

    irc_connect(&irc, "irc.freenode.net", "6666");
    irc_login(&irc, "srainbot");
    irc_join_chan(&irc, "#lasttest");
    for (;;){
        irc_recv(&irc, nick, chan, cmd, msg);
        // LOG_FR("nick = %s chan = %s cmd = %s msg = %s", nick, chan, cmd, msg);
    }
    return 0;
}
