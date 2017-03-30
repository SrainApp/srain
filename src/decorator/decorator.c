// #define __DBG_ON
// #define __LOG_ON

#include <glib.h>
#include <string.h>

#include "decorator.h"

#include "srain.h"
#include "log.h"

#define MAX_DECORATOR   32  // Bits of a DecoratorFlag(int)

extern Decorator relay_decroator;
extern Decorator mirc_strip_decroator;
extern Decorator pango_markup_decroator;

static Decorator *decorators[MAX_DECORATOR];

void decorator_init(){
    memset(decorators, 0, sizeof(decorators));

    decorators[0] = &relay_decroator;
    decorators[2] = &mirc_strip_decroator;
    decorators[3] = &pango_markup_decroator;
}

int decorate_message(Message *msg, DecoratorFlag flag, void *user_data){
    int ret;

    for (int i = 0; i < MAX_DECORATOR; i++){
        if ((flag & (1 << i))
                && decorators[i]
                && decorators[i]->name
                && decorators[i]->func){

            DBG_FR("Run decorator '%s' for message %p", decorators[i]->name, msg);

            ret = decorators[i]->func(msg, flag, user_data);
            if (ret != SRN_OK){
                ERR_FR("Decorator '%s' return %d for message %p",
                        decorators[i]->name, ret, msg);

                return ret;
            }
        } else {
            // DBG_FR("No available decorator for bit %d", i);
        }
    }

    return SRN_OK;
}

char* decorate_content(const char *content, DecoratorFlag flag){
    char *dcontent;

    Message *msg = message_new(NULL, NULL, content);

    if (decorate_message(msg, flag, NULL) == SRN_OK){
        dcontent = g_strdup(msg->dcontent);
    } else {
        dcontent = NULL;
    }

    message_free(msg);

    return dcontent;
}
