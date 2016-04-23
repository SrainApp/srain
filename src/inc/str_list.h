#ifndef __STR_LIST_H
#define __STR_LIST_H

#include <glib.h>

GList* str_list_find(GList *lst, const char *str);
GList* str_list_add(GList *lst, const char *str);
GList* str_list_rm(GList *lst, const char *str);

#endif /* __STR_LIST_H */
