#define __DBG_ON
#define __LOG_ON

#include <glib.h>

#include "decorator.h"

#include "srain.h"
#include "log.h"

extern Decorator mirc_strip_decroator;

int decorate_message(Message *msg, DecoratorFlag flag, void *user_data){
    int ret;

    if (flag & DECORATOR_MIRC_STRIP) {
        if (mirc_strip_decroator.func) {
            DBG_FR("Run decorator %s for message %p", mirc_strip_decroator.name, msg);

            ret = mirc_strip_decroator.func(msg, flag, user_data);
            if (ret != SRN_OK){
                ERR_FR("Decorator %s return %d for message %p",
                        mirc_strip_decroator.name, ret, msg);
            }
        }
    }

    return SRN_OK;
}
