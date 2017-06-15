#include <glib.h>
#include <strings.h>

#include "server.h"

#include "srain.h"
#include "log.h"

static GSList *server_list = NULL;

bool server_list_is_server(Server *srv){
    g_return_val_if_fail(srv, FALSE);

    return g_slist_find(server_list, srv);
}

Server *server_list_get_server(const char *name){
    GSList *lst;
    Server *srv;

    g_return_val_if_fail(name, NULL);

    lst = server_list;
    while (lst){
        srv = lst->data; 
        if (strcasecmp(srv->prefs->name, name) == 0){
            return srv;
        }
        lst = g_slist_next(lst);
    }

    return NULL;
}

int server_list_add(Server *srv){
    g_return_val_if_fail(srv, SRN_ERR);
    g_return_val_if_fail(!server_list_get_server(srv->prefs->name), SRN_ERR);

    server_list = g_slist_append(server_list, srv);

    return SRN_OK;
}

int server_list_rm(Server *srv){
    g_return_val_if_fail(server_list_is_server(srv), SRN_ERR);

    server_list = g_slist_remove(server_list, srv);

    return SRN_ERR;
}
