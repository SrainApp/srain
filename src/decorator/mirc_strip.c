#include "decorator.h"

#include "srain.h"
#include "log.h"

static int mirc_stirp(Message *msg, DecoratorFlag flag, void *user_data);

Decorator mirc_strip_decroator = {
    .name = "mirc_strip",
    .func = mirc_stirp,
};

int mirc_stirp(Message *msg, DecoratorFlag flag, void *user_data){
    int i;
    int j;
    int len;
    char *str;

    str = msg->dcontent;
    LOG_FR(": %s", str);

    j = 0;
    len = strlen(str);

    for (i = 0; i < len; i++){
        switch (str[i]){
            case 2: case 0xf: case 0x16:
            case 0x1d: case 0x1f:
                break;
            case 3:  // irc color code
                if (str[i+1] >= '0' && str[i+1] <= '9'){
                    if (str[i+2] >= '0' && str[i+2] <= '9'){
                        i += 2;
                    } else {
                        i += 1;
                    }
                }
                break;
            default:
                str[j++] = str[i];
        }
    }

    str[j] = '\0';

    return SRN_OK;
}
