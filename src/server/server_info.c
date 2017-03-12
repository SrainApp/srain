#include "server.h"

ServerInfo *server_info_new(const char *name, const char *host, int port,
        const char *passwd, bool ssl, const char *encoding){
    ServerInfo *info;

    if (!host) return NULL;
    if (!name) name = host;
    if (!passwd) passwd = "";
    if (!encoding) encoding = "UTF-8";

    info = g_malloc0(sizeof(ServerInfo));

    info->port = port;
    info->ssl = ssl;
    info->encoding = encoding;

    g_strlcpy(info->name, name, sizeof(info->name));
    g_strlcpy(info->host, host, sizeof(info->host));
    g_strlcpy(info->passwd, passwd, sizeof(info->passwd));

    return info;
}

void server_info_free(ServerInfo *info){
    g_free(info);
}
