#include <glib.h>

#include "server.h"

#include "sirc/sirc.h"

ServerPrefs* server_prefs_new(){
    ServerPrefs *prefs;
    SircPrefs *irc_prefs;

    prefs = g_malloc0(sizeof(ServerPrefs));
    irc_prefs = sirc_prefs_new();

    prefs->irc = irc_prefs;

    return prefs;
}

bool server_prefs_is_valid(ServerPrefs *prefs){
    return (prefs &&
            prefs->name
            && prefs->host
            && prefs->passwd
            && prefs->encoding
            && prefs->nickname
            && prefs->username
            && prefs->realname
            && prefs->part_message
            && prefs->kick_message
            && prefs->away_message
            && prefs->quit_message
            && prefs->irc);
}

void server_prefs_free(ServerPrefs *prefs){
    if (prefs->name){
        g_free(prefs->name);
        prefs->name = NULL;
    }

    if (prefs->host){
        g_free(prefs->host);
        prefs->host = NULL;
    }

    if (prefs->passwd){
        g_free(prefs->passwd);
        prefs->passwd = NULL;
    }

    if (prefs->encoding){
        g_free(prefs->encoding);
        prefs->encoding = NULL;
    }

    if (prefs->passwd){
        g_free(prefs->passwd);
        prefs->passwd = NULL;
    }

    if (prefs->encoding){
        g_free(prefs->encoding);
        prefs->encoding = NULL;
    }

    if (prefs->nickname){
        g_free(prefs->nickname);
        prefs->nickname = NULL;
    }

    if (prefs->username){
        g_free(prefs->username);
        prefs->username = NULL;
    }

    if (prefs->realname){
        g_free(prefs->realname);
        prefs->realname = NULL;
    }

    if (prefs->part_message){
        g_free(prefs->part_message);
        prefs->part_message = NULL;
    }

    if (prefs->kick_message){
        g_free(prefs->kick_message);
        prefs->kick_message = NULL;
    }

    if (prefs->away_message){
        g_free(prefs->away_message);
        prefs->away_message = NULL;
    }

    if (prefs->quit_message){
        g_free(prefs->quit_message);
        prefs->quit_message = NULL;
    }

    if (prefs->irc){
        sirc_prefs_free(prefs->irc);
        prefs->irc = NULL;
    }
}
