#ifndef __UI_H
#define __UI_H

/*
 * */
typedef struct {
    char *id;
    char *nick;
    char *chan;
    char *msg;
    char *time;
    char *avatar;   // path of cached avatar, can be null
    char *img;      // path of cached img, can be null
} bubble_msg_t;

void ui_window_init();

int ui_chat_add(const char *name, const char *topic);
int ui_chat_rm(const char *name);
int ui_chat_set_topic(const char *name, const char *topic);
int ui_online_list_add(const char *chat_name, const char *nick);
int ui_online_list_rm(const char *chat_name, const char *nick);

void ui_msg_init();
int ui_msg_send(const bubble_msg_t *msg);
int ui_msg_recv(const bubble_msg_t *msg);
int ui_msg_sys(const bubble_msg_t *msg);

#endif /* __UI_H */
