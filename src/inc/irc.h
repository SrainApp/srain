#ifndef __IRC_H
#define __IRC_H

void irc_init();
int irc_session_new(const char *host, int port, const char *password, 
        const char *nickname, const char *username, const char *realname);
int irc_session_process();

#endif /* __IRC_H */
