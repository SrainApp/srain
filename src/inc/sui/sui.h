#ifndef __SUI_H
#define __SUI_H

typedef struct _SuiSession SuiSession;
typedef int SuiSessionFlag;

#define SUI_SESSION_SERVER      1 << 0
#define SUI_SESSION_CHANNEL     1 << 1
#define SUI_SESSION_DIALOG      1 << 2
#define SUI_SESSION_NEWWINDOW   1 << 3  // Not used yet
#define SUI_SESSION_NOSWITCH    1 << 4  // Not used yet

// TODO Rename type
typedef enum {
    SYS_MSG_NORMAL,
    SYS_MSG_ERROR,
    SYS_MSG_ACTION
} SysMsgType;

typedef enum {
    USER_CHIGUA,    // No prefix
    USER_OWNER,     // ~ mode +q
    USER_ADMIN,     // & mode +a
    USER_FULL_OP,   // @ mode +o
    USER_HALF_OP,   // % mode +h
    USER_VOICED,    // + mode +v
    /* ... */
    USER_TYPE_MAX
} UserType;

typedef int SrainMsgFlag;

#define SRAIN_MSG_MENTIONED 0x1

#define __IN_SUI_H
#include "sui_event.h"
#undef __IN_SUI_H

void sui_main_loop(SuiAppEvents *events);
void sui_proc_pending_event();

SuiSession *sui_new_session(SuiEvents *events, SuiSessionFlag flag);
void sui_free_session(SuiSession *sui);
int sui_start_session(SuiSession *sui, const char *name, const char *remark);
void sui_end_session(SuiSession *sui);

SuiSessionFlag sui_get_flag(SuiSession *sui);
SuiEvents *sui_get_events(SuiSession *sui);
void* sui_get_ctx(SuiSession *sui);
void sui_set_ctx(SuiSession *sui, void *ctx);
void sui_set_name(SuiSession *sui, const char *name);
void sui_set_remark(SuiSession *sui, const char *remark);

void sui_add_sys_msg(SuiSession *sui, const char *msg, SysMsgType type, SrainMsgFlag flag);
void sui_add_sys_msgf(SuiSession *sui, SysMsgType type, SrainMsgFlag flag,
        const char *fmt, ...);
void sui_add_sent_msg(SuiSession *sui, const char *msg, SrainMsgFlag flag);
void sui_add_recv_msg(SuiSession *sui, const char *nick, const char *id,
        const char *msg, SrainMsgFlag flag);

int sui_add_user(SuiSession *sui, const char *nick, UserType type);
int sui_rm_user(SuiSession *sui, const char *nick);
int sui_ren_user(SuiSession *sui, const char *old_nick, const char *new_nick, UserType type);
void sui_set_topic(SuiSession *sui, const char *topic);

#endif /* __SUI_H */
