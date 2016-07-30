/**
 * @file srv_name.c
 * @brief SRV module interfaces
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-07-19
 */

#define __LOG_ON
#define __DBG_ON

#include "srv_session.h"
#include "srv_session_cmd.h"
#include "srv_hdr.h"

#include "meta.h"
#include "log.h"

/**
 * @brief Initilize srv_session and start networking loop
 */
void srv_init(){
    srv_hdr_init();
    srv_session_init();
    srv_session_proc();
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
int srv_connect(const char *host, int port, const char *passwd,
        const char *nickname, const char *username, const char *realname){
    int res;

    if (!host || !nickname){
        ERR_FR("host or nickname is NULL");
        return -1;
    }

    res = srv_session_new(host, port, passwd, nickname, username, realname)
        ?  0 : -1;
    // if (res == 0){
        // srv_hdr_ui_add_chan(host, SRV_SESSION_SERVER);
    // }

    return res;
}

int srv_send(const char *srv_name, const char *target, const char *msg){
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

int srv_cmd(const char *srv_name, const char *source, char *cmd){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);

    return srv_session_cmd(session, source, cmd);
}

int srv_join(const char *srv_name, const char *chan, const char *passwd){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_join(session, chan, passwd);
}

int srv_part(const char *srv_name, const char *chan, const char *reason){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_part(session, chan);
}

int srv_quit(const char *srv_name, const char *reason){
    srv_session_t *session;

    session = srv_session_get_by_host(srv_name);
    if (!session){
        WARN_FR("No such session %s", srv_name);
        return -1;
    }

    return srv_session_quit(session, reason);
}
