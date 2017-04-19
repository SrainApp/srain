#ifndef __DECORATOR_H
#define __DECORATOR_H

#include "server.h"

#define DECORATOR_RELAY         1 << 0
#define DECORATOR_MIRC_COLORIEZ 1 << 1
#define DECORATOR_MIRC_STRIP    1 << 2
#define DECORATOR_PANGO_MARKUP  1 << 3
#define DECORATOR_MENTION       1 << 4

typedef int DecoratorFlag;

typedef int (DecoratorFunc) (Message *msg, DecoratorFlag flag, void *user_data);;

typedef struct _Decorator {
    const char *name;
    DecoratorFunc *func;
} Decorator;

void decorator_init();
int decorate_message(Message *msg, DecoratorFlag flag, void *user_data);
// char* decorate_content(const char *content, DecoratorFlag flag);

int relay_decroator_add_nick(Chat *chat, const char *nick);
int relay_decroator_rm_nick(Chat *chat, const char *nick);
void relay_decroator_free_list(Chat *chat);

#endif /* __DECORATOR_H */
