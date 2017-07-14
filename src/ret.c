/**
 * @file ret.c
 * @brief Srain return value, which can carry message
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-07-13
 */

#include <glib.h>

#include "i18n.h"
#include "ret.h"

#define SRN_RET_MESSAGE_COUNT 512

typedef struct _SrnRetMessage SrnRetMessage;

struct _SrnRetMessage {
    SrnRet id;
    int no;
    char *msg;
};

static SrnRet msgid = 0;
static GSList *msg_list = NULL;
static GMutex mutex;

static SrnRetMessage* srn_ret_message_new(SrnRet id, int no, const char *msg);
static void srn_ret_message_free(SrnRetMessage *rmsg);
static SrnRet ret_with_message(int no, const char *fmt, va_list ap);

void ret_init(){
    g_mutex_init(&mutex);
}

void ret_finalize(){
    g_slist_free_full(msg_list, (GDestroyNotify)srn_ret_message_free);
}

SrnRet ret_err(const char *fmt, ...){
    SrnRet id;
    va_list args;

    va_start(args, fmt);
    id = ret_with_message(SRN_ERR, fmt, args);
    va_end(args);

    return id;
}

SrnRet ret_ok(const char *fmt, ...){
    SrnRet id;
    va_list args;

    va_start(args, fmt);
    id = ret_with_message(SRN_OK, fmt, args);
    va_end(args);

    return id;
}

const char *ret_get_message(SrnRet id){
    const char *msg;
    GSList *lst;
    SrnRetMessage *err;

    g_mutex_lock(&mutex);

    msg = NULL;

    if (id == SRN_OK) {
        goto fin;
    }
    if (id == SRN_ERR) {
        msg = _("Some error occurred");
        goto fin;
    }

    lst = msg_list;
    while (lst){
        err = lst->data;

        if (err->id == id) {
            msg = err->msg;
            break;
        }
        lst = g_slist_next(lst);
    }

fin:
    g_mutex_unlock(&mutex);
    if (id != SRN_OK && !msg){
        msg = _("Invalid error id, maybe this error is removed because out of date");
    }

    return msg;
}

int ret_get_no(SrnRet id){
    int no;
    GSList *lst;
    SrnRetMessage *rmsg;

    no = SRN_ERR;
    g_mutex_lock(&mutex);

    if (id == SRN_OK || id == SRN_ERR){
        no = id;
        goto fin;
    }

    rmsg = NULL;
    lst = msg_list;
    while (lst){
        rmsg = lst->data;
        if (rmsg->id == id) {
            no = rmsg->no;
            break;
        }
        lst = g_slist_next(lst);
    }

fin:
    g_mutex_unlock(&mutex);
    return no;
}

static SrnRet ret_with_message(int no, const char *fmt, va_list ap){
    GString *msg;
    SrnRetMessage *rmsg;

    g_mutex_lock(&mutex);

    if (g_slist_length(msg_list) > SRN_RET_MESSAGE_COUNT){
        // If msg_list full
        GSList *last;
        last = g_slist_last(msg_list);
        srn_ret_message_free((SrnRetMessage *)last->data);
        msg_list = g_slist_delete_link(msg_list, last);
    }

    msg = g_string_new(NULL);
    g_string_append_vprintf(msg, fmt, ap);

    rmsg = srn_ret_message_new(++msgid, no, msg->str);
    g_string_free(msg, TRUE);

    msg_list = g_slist_prepend(msg_list, rmsg);

    g_mutex_unlock(&mutex);

    return rmsg->id;
}

static SrnRetMessage* srn_ret_message_new(SrnRet id, int no, const char *msg){
    SrnRetMessage *rmsg;

    rmsg = g_malloc0(sizeof(SrnRetMessage));
    rmsg->id = id;
    rmsg->msg = g_strdup(msg);

    return rmsg;
}

static void srn_ret_message_free(SrnRetMessage *rmsg){
    g_free(rmsg->msg);
    g_free(rmsg);
}
