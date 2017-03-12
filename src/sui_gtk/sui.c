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

struct _SuiSession{
    SrainChat *chat;

    SuiSessionFlag flag;
    SuiEvents *events;
    void *ctx;
};

SuiAppEvents *app_events = NULL;

void sui_main_loop(SuiAppEvents *events){
    g_return_if_fail(events);

    app_events = events;

    theme_init();
    snotify_init();

    g_application_run(G_APPLICATION(srain_app_new()), 0, NULL);

    snotify_finalize();
}

void sui_proc_pending_event(){
    while (gtk_events_pending()) gtk_main_iteration();
}

SuiSession *sui_new_session(SuiEvents *events, SuiSessionFlag flag){
    SuiSession *sui;

    g_return_val_if_fail(events, NULL);

    sui = g_malloc0(sizeof(SuiSession));

    // sui->chat = NULL; // via g_malloc0()
    sui->flag = flag;
    sui->events = events;

    return sui;
}

void sui_free_session(SuiSession *sui){
    g_return_if_fail(sui);

    if (sui->chat){
        srain_window_rm_chat(srain_win, sui->chat);
    }
    g_free(sui);
}

int sui_start_session(SuiSession *sui, const char *name, const char *remark){
    ChatType type;
    SuiSessionFlag flag;

    g_return_val_if_fail(sui, SRN_ERR);
    g_return_val_if_fail(name, SRN_ERR);
    g_return_val_if_fail(remark, SRN_ERR);

    flag = sui->flag;
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
        return SRN_ERR;
    }

    sui->chat = srain_window_add_chat(srain_win, sui, name, remark, type);

    if (!sui->chat){
        return SRN_ERR;
    }

    return SRN_OK;
}

void sui_end_session(SuiSession *sui){
    g_return_if_fail(sui);

    srain_window_rm_chat(srain_win, sui->chat);
    sui->chat = NULL;
}

SuiSessionFlag sui_get_flag(SuiSession *sui){
    /* Don't return SRN_ERR(-1 0xffffffff) */
    g_return_val_if_fail(sui, 0);

    return sui->flag;
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

void sui_set_name(SuiSession *sui, const char *name){
    g_return_if_fail(sui);
    g_return_if_fail(name);

    srain_chat_set_name(sui->chat, name);
}

void sui_set_remark(SuiSession *sui, const char *remark){
    g_return_if_fail(sui);
    g_return_if_fail(remark);

    srain_chat_set_remark(sui->chat, remark);
}

void sui_add_sys_msg(SuiSession *sui, const char *msg, SysMsgType type, SrainMsgFlag flag){
    SrainChat *chat;
    SrainMsgList *list;

    g_return_if_fail(sui);
    g_return_if_fail(msg);

    chat = sui->chat;
    if (!chat){ // TODO: remove it ?
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

    g_return_if_fail(sui);
    g_return_if_fail(fmt);

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf) - 1, fmt, args);
    va_end(args);

    sui_add_sys_msg(sui, buf, type, flag);
}

void sui_add_sent_msg(SuiSession *sui, const char *msg, SrainMsgFlag flag){
    SrainChat *chat;
    SrainMsgList *list;

    g_return_if_fail(msg);

    g_return_if_fail(sui);
    chat = sui->chat;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    list = srain_chat_get_msg_list(chat);

    srain_msg_list_send_msg_add(list, msg, flag);
    srain_window_stack_sidebar_update(srain_win, chat, _("You"), msg);
}

void sui_add_recv_msg(SuiSession *sui, const char *nick, const char *id,
        const char *msg, SrainMsgFlag flag){
    SrainChat *chat;
    SrainMsgList *list;
    SrainEntryCompletion *comp;

    g_return_if_fail(nick);
    g_return_if_fail(msg);
    if (!id) id = "";

    g_return_if_fail(sui);
    chat = sui->chat;

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
    SrainChat *chat;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(nick, SRN_ERR);

    g_return_val_if_fail(sui, SRN_ERR);
    chat = sui->chat;

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
    SrainChat *chat;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(nick, SRN_ERR);

    g_return_val_if_fail(sui, SRN_ERR);
    chat = sui->chat;

    g_return_val_if_fail(SRAIN_IS_CHAT(chat), SRN_ERR);

    list = srain_chat_get_user_list(chat);

    if ((res = srain_user_list_rm(list, nick)) == 0){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_rm_keyword(comp, nick);
    }

    return res;
}

int sui_ren_user(SuiSession *sui, const char *old_nick, const char *new_nick,
        UserType type){
    int ret;
    SrainChat *chat;
    SrainUserList *list;
    SrainEntryCompletion *comp;

    g_return_val_if_fail(old_nick, SRN_ERR);
    g_return_val_if_fail(new_nick, SRN_ERR);

    g_return_val_if_fail(sui, SRN_ERR);
    chat = sui->chat;

    g_return_val_if_fail(SRAIN_IS_CHAT(chat), SRN_ERR);

    /* Your nick changed */
    if (strcmp(old_nick, srain_chat_get_nick(chat)) == 0){
        srain_chat_set_nick(chat, new_nick);
    }

    list = srain_chat_get_user_list(chat);

    ret = srain_user_list_rename(list, old_nick, new_nick, type);

    if (ret == SRN_OK){
        comp = srain_chat_get_entry_completion(chat);
        srain_entry_completion_add_keyword(comp, old_nick, KEYWORD_NORMAL);
        srain_entry_completion_rm_keyword(comp, new_nick);
    }

    return ret;
}

void sui_set_topic(SuiSession *sui, const char *topic){
    SrainChat *chat;

    g_return_if_fail(sui);
    chat = sui->chat;

    g_return_if_fail(SRAIN_IS_CHAT(chat));

    srain_chat_set_topic(chat, topic);
}
