/**
 * @file filter.c
 * @brief Message filter
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-03-16
 */

#define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <string.h>

#include "filter.h"

#include "srain.h"
#include "log.h"

#define MAX_FILTER   32  // Bits of a FilterFlag(int)

// extern Filter nick_filter;
// extern Filter regex_filter;

static Filter *filters[MAX_FILTER];

void filter_init(){
    memset(filters, 0, sizeof(filters));

    // filters[0] = &nick_filter;
    // filters[1] = &regex_filter;
}

bool filter_message(const Message *msg, FilterFlag flag, void *user_data){
    int ret;

    for (int i = 0; i < MAX_FILTER; i++){
        if ((flag & (1 << i))
                && filters[i]
                && filters[i]->name
                && filters[i]->func){

            DBG_FR("Run filter '%s' for message %p", filters[i]->name, msg);

            ret = filters[i]->func(msg, flag, user_data);
            if (ret != SRN_OK){
                ERR_FR("Filter '%s' return %d for message %p",
                        filters[i]->name, ret, msg);

                return ret;
            }
        } else {
            // DBG_FR("No available filter for bit %d", i);
        }
    }

    return SRN_OK;
}
