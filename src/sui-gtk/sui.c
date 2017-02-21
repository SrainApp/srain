/* @file sui.c
 * @brief GTK UI module interfaces
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-06-29
 */

#define __LOG_ON
#define __DBG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "sui/sui.h"

#include "sui_common.h"
#include "srain_app.h"
#include "srain_window.h"
#include "theme.h"
#include "snotify.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"
#include "meta.h"

#define SUI_NAME_LEN 128

struct _SuiSession{
    char name[SUI_NAME_LEN];
    char remark[SUI_NAME_LEN];
    SrainChat *ui;

    ChatType type;
    SuiEvents *events;
    void *ctx;
};

void sui_main_loop(int argc, char **argv){
    theme_init();
    snotify_init();

    g_application_run(G_APPLICATION(srain_app_new()), argc, argv);

    snotify_finalize();
}

void sui_proc_pending_event(){
    while (gtk_events_pending()) gtk_main_iteration();
}

SuiSession *sui_new_session(const char *name, const char *remark,
        SuiEvents *events, SuiSessionFlag flag){
    ChatType type;
    SuiSession *sui;

    g_return_val_if_fail(name, NULL);
    g_return_val_if_fail(remark, NULL);

    sui = g_malloc0(sizeof(SuiSession));

    if (flag & SUI_SESSION_SERVER) {
        type = CHAT_SERVER;
    }
    else if (flag & SUI_SESSION_CHANNEL) {
        type = CHAT_CHANNEL;
    }
    else if (flag & SUI_SESSION_DIALOG) {
        type = CHAT_PRIVATE;
    } else {
        ERR_FR("Chat type not found in SuiSessionFlag 0x%x", flag);
    }

    sui->ui = srain_window_add_chat(srain_win, sui, name, remark, type);
    sui->events = events;

    if (!sui->ui){
        goto bad;
    }

    g_strlcpy(sui->name, name, sizeof(sui->name));
    g_strlcpy(sui->remark, remark, sizeof(sui->remark));

    return sui;

bad:
    if (sui) {
        sui_free_session(sui);
    }
    return NULL;
}

void sui_free_session(SuiSession *sui){
    g_return_if_fail(sui);

    if (sui->ui){
        srain_window_rm_chat(srain_win, sui->ui);
    }
    g_free(sui);
}

SuiEvents *sui_get_events(SuiSession *sui){
    g_return_val_if_fail(sui, NULL);

    return sui->events;
}

void* sui_get_ctx(SuiSession *sui){
    g_return_val_if_fail(sui, NULL);

    return sui->ctx;
}

void sui_set_ctx(SuiSession *sui, void *ctx){
    g_return_if_fail(sui);

    sui->ctx = ctx;
}

void sui_add_sys_msg(SuiSession *sui, const char *msg, SysMsgType type, SrainMsgFlag flag){
    SrainChat *chat = sui->ui;
    SrainMsgList *list;

    if (!chat){
        if (type == SYS_MSG_ERROR){
            char buf[512];
            snprintf(buf, sizeof(buf), _("<b>Session:</b> %s\n<b>Error message:</b> %s"),
                    "TODO", msg);
            show_msg_dialog(_("ERROR"), buf);
        }
        return;
    }

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    list = srain_chat_get_msg_list(chat);
    srain_msg_list_sys_msg_add(list, msg, type, flag);

    if (type != SYS_MSG_NORMAL){
        srain_window_stack_sidebar_update(srain_win, chat, NULL, msg);
    }
}

void sui_add_sys_msgf(SuiSession *sui, SysMsgType type, SrainMsgFlag flag,
        const char *fmt, ...){
    char buf[512];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    sui_add_sys_msg(sui, buf, type, flag);
}

void ui_add_sent_msg(SuiSession *sui, const char *msg, SrainMsgFlag flag){
    SrainChat *chat = sui->ui;
    SrainMsgList *list;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    list = srain_chat_get_msg_list(chat);

    srain_msg_list_send_msg_add(list, msg, flag);
    srain_window_stack_sidebar_update(srain_win, chat, _("You"), msg);
}

void sui_add_recv_msg(SuiSession *sui, const char *nick, const char *id,
        const char *msg, SrainMsgFlag flag){
    SrainChat *chat = sui->ui;
    SrainMsgList *list;
    SrainEntryCompletion *comp;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    list = srain_chat_get_msg_list(chat);
    srain_msg_list_recv_msg_add(list, nick, id, msg, flag);

    // TODO: move in srain_msg_list?
    srain_window_stack_sidebar_update(srain_win, chat, nick, msg);
    if (strlen(id) != 0){
        comp = srain_chat_get_entry_completion(chat);
        if (!comp) return;
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_TMP);
    }
}

int sui_add_user(SuiSession *sui, const char *nick, UserType type){
    int res;
    SrainChat *chat = sui->ui;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(SRAIN_IS_CHAT(chat), SRN_ERR);

    list = srain_chat_get_user_list(chat);

    if ((res = srain_user_list_add(list, nick, type)) == 0){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_add_keyword(comp, nick, KEYWORD_NORMAL);
    };

    return res;
}

int sui_rm_user(SuiSession *sui, const char *nick){
    int res;
    SrainChat *chat = sui->ui;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(SRAIN_IS_CHAT(chat), SRN_ERR);

    list = srain_chat_get_user_list(chat);

    if ((res = srain_user_list_rm(list, nick)) == 0){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return res;
}

void sui_ren_user(SuiSession *sui, const char *old_nick, const char *new_nick,
        UserType type){
    SrainChat *chat = sui->ui;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    /* Your nick changed */
    if (strcmp(old_nick, srain_chat_get_nick(chat)) == 0){
        srain_chat_set_nick(chat, new_nick);
    }

    list = srain_chat_get_user_list(chat);

    if (srain_user_list_rename(list, old_nick, new_nick, type) == 0){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }
}

void sui_set_topic(SuiSession *sui, const char *topic){
    SrainChat *chat = sui->ui;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    srain_chat_set_topic(chat, topic);
}
