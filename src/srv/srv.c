/**
 * @file srv_name.c
 * @brief SRV module interfaces
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __LOG_ON
// #define __DBG_ON

#include <string.h>

#include "srv_session.h"
#include "srv_session_cmd.h"
#include "srv_hdr.h"
#include "srv_test.h"

#include "meta.h"
#include "log.h"

/**
 * @brief Initilize srv_session and start networking loop
 */
void srv_init(){
    srv_hdr_init();
    srv_session_cmd_init();
    srv_session_init();
    srv_session_proc();

#ifdef IRC_TEST
    srv_test();
    exit(0);
#endif
}

void srv_finalize(){
    LOG_FR("...");
    srv_session_quit_all();
}

/**
 * @brief Connect to a IRC server
 *
 * @param host host of server
 * @param port If port = 0, use default port
 * @param passwd Server password, can be NULL
 * @param nickname
 * @param username Can be NULL
 * @param realname Can be NULL
 *
 * @return 0 if successed, -1 if failed
 */
int srv_connect(SIGN_SRV_CONNECT){
    int res;

    if (!host || !nickname){
        ERR_FR("host or nickname is NULL");
        return -1;
    }

    res = srv_session_new(host, port, passwd, nickname, username, realname, ssl)
        ?  0 : -1;

    return res;
}

int srv_quit(SIGN_SRV_QUIT){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_quit(session, reason);
}


int srv_cmd(SIGN_SRV_CMD){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);

    return srv_session_cmd(session, source, cmd, block);
}

int srv_query(SIGN_SRV_QUERY){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    if (IS_CHAN(nick)){
        return -1;
    }

    srv_hdr_ui_add_chat(session->host, nick, session->nickname, CHAT_PRIVATE);

    return 0;
}

int srv_unquery(SIGN_SRV_UNQUERY){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    if (IS_CHAN(nick)){
        return -1;
    }

    srv_hdr_ui_rm_chat(session->host, nick);

    return 0;
}

int srv_join(SIGN_SRV_JOIN){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    if (!IS_CHAN(chan)){
        return -1;
    }

    return srv_session_join(session, chan, passwd);
}

int srv_part(SIGN_SRV_PART){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    if (!IS_CHAN(chan)){
        return -1;
    }

    return srv_session_part(session, chan);
}

int srv_send(SIGN_SRV_SEND){
    srv_session_t *session;

    if (!srv_name || !target || !msg){
        ERR_FR("host or nickname is NULL");
        return -1;
    }

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_send(session, target, msg);
}

int srv_kick(SIGN_SRV_KICK){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_kick(session, nick, chan, reason);
}

int srv_whois(SIGN_SRV_WHOIS){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_whois(session, nick);
}

int srv_invite(SIGN_SRV_INVITE){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_invite(session, nick, chan);
}
