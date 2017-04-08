#ifndef __FILTER_H
#define __FILTER_H

#include "srain.h"
#include "server.h"

#define FILTER_NICK         1 << 0
#define FILTER_REGEX        1 << 1
#define FILTER_CHAT_LOG     1 << 2

typedef int FilterFlag;

typedef bool (FilterFunc) (const Message *msg, FilterFlag flag, void *user_data);;

typedef struct _Filter {
    const char *name;
    FilterFunc *func;
} Filter;

void filter_init();
bool filter_message(const Message *msg, FilterFlag flag, void *user_data);

int nick_filter_add_nick(Chat *chat, const char *nick);
int nick_filter_rm_nick(Chat *chat, const char *nick);
void nick_filter_free_nick_list(Chat *chat);

int regex_filter_add_pattern(Chat *chat, const char *name, const char *pattern);
int regex_filter_rm_pattern(Chat *chat, const char *name);

#endif /* __FILTER_H */
