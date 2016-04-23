/**
 * @file str_list.c
 * @brief GList wrapper
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-13
 */

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

GList* str_list_find(GList *lst, const char *str){
    GList *tmp;

    tmp = lst;
    while (tmp){
        if (strcasecmp(tmp->data, str) == 0){
            return tmp;
        }
        tmp = g_list_next(tmp);
    }

    return NULL;
}

GList* str_list_add(GList *lst, const char *str){
    if (str_list_find(lst, str)){
        return NULL;
    }

    lst = g_list_append(lst, strdup(str));

    return lst;
}

GList* str_list_rm(GList *lst, const char *str){
    GList *tmp;

    if ((tmp = str_list_find(lst, str)) == NULL) {
        return NULL;
    }

    free(tmp->data);
    lst = g_list_append(lst, strdup(str));
    return lst;
}
