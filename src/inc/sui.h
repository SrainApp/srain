#ifndef __SUI_H
#define __SUI_H

#include "srain_chat.h"
#include "srain_msg.h"
#include "srain_user_list.h"

#define SUI_NAME_LEN 128

typedef struct _SuiSession SuiSession;

typedef struct {

} SuiEvents;

struct _SuiSession{
    char name[SUI_NAME_LEN];
    char remark[SUI_NAME_LEN];
    SrainChat *ui;

    SuiEvents events;
    void *ctx;
} ;

void sui_main_loop(int argc, char **argv);
SuiSession *sui_new_session(const char *name, const char *remark, ChatType type, void *ctx);
void sui_free_session(SuiSession *sui);
void* sui_get_ctx(SuiSession *sui);
void sui_set_ctx(SuiSession *sui, void *ctx);

void sui_add_sys_msg(SuiSession *sui, const char *msg, SysMsgType type, SrainMsgFlag flag);
void sui_add_sys_msgf(SuiSession *sui, SysMsgType type, SrainMsgFlag flag,
        const char *fmt, ...);
void ui_add_sent_msg(SuiSession *sui, const char *msg, SrainMsgFlag flag);
void sui_add_recv_msg(SuiSession *sui, const char *nick, const char *id,
        const char *msg, SrainMsgFlag flag);

int sui_add_user(SuiSession *sui, const char *nick, UserType type);
int sui_rm_user(SuiSession *sui, const char *nick);
void sui_ren_user(SuiSession *sui, const char *old_nick, const char *new_nick, UserType type);
void sui_set_topic(SuiSession *sui, const char *topic);

#endif /* __SUI_H */
