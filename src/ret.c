/**
 * @file ret.c
 * @brief Srain return value, support for returning error message
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2017-07-13
 */

#include <glib.h>

#include "i18n.h"
#include "ret.h"

#define SRN_RET_ERR_COUNT 512

typedef struct _SrnErr SrnErr;

struct _SrnErr {
    int id;
    char *msg;
};

static int errid = 0;
static GSList *err_list = NULL;
static GMutex mutex;

static SrnErr* srn_err_new(int id, const char *msg);
static void srn_err_free(SrnErr *err);

void ret_init(){
    g_mutex_init(&mutex);
}

void ret_finalize(){
    g_slist_free_full(err_list, (GDestroyNotify)srn_err_free);
}

SrnRet ret_err(const char *fmt, ...){
    int id;
    va_list args;
    GString *msg;
    SrnErr *err;

    g_mutex_lock(&mutex);

    if (g_slist_length(err_list) > SRN_RET_ERR_COUNT){
        // If err_list full
        GSList *last;
        last = g_slist_last(err_list);
        srn_err_free((SrnErr *)last->data);
        err_list = g_slist_delete_link(err_list, last);
    }

    msg = g_string_new(NULL);
    va_start(args, fmt);
    g_string_append_vprintf(msg, fmt, args);
    va_end(args);

    err = srn_err_new(++errid, msg->str);
    g_string_free(msg, TRUE);

    err_list = g_slist_prepend(err_list, err);

    g_mutex_unlock(&mutex);

    return err->id;
}

const char *ret_errmsg(int id){
    const char *msg;
    GSList *lst;
    SrnErr *err;

    g_mutex_lock(&mutex);

    if (id == SRN_OK) {
        msg =  _("No error occurred");
        goto fin;
    }
    if (id == SRN_ERR) {
        msg = _("Some error occurred");
        goto fin;
    }

    msg = NULL;
    lst = err_list;
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
    if (!msg){
        msg = _("Invalid error id, maybe this error is removed because out of date");
    }

    return msg;
}

static SrnErr* srn_err_new(int id, const char *msg){
    SrnErr *err;

    err = g_malloc0(sizeof(SrnErr));
    err->id = id;
    err->msg = g_strdup(msg);

    return err;
}

static void srn_err_free(SrnErr *err){
    g_free(&err->msg);
    g_free(err);
}
