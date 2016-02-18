#ifndef __UI_H
#define __UI_H

#include <glib.h>
#include <irc.h>

#define PATH_LEN 256

typedef struct {
    char name[CHAN_LEN];
} chan_name_t;

typedef struct {
    char chan[CHAN_LEN];
    char nick[NICK_LEN];
} online_list_item_t;

typedef struct {
    char chan[CHAN_LEN];
    char topic[MSG_LEN];
} topic_t;

typedef struct {
    char id[NICK_LEN];
    char nick[NICK_LEN];
    char chan[CHAN_LEN];
    char msg[MSG_LEN];
    char avatar[PATH_LEN];   // path of cached avatar, can be null
    char img[PATH_LEN];      // path of cached img, can be null
} recv_msg_t;

typedef struct {
    char chan[CHAN_LEN];
    char msg[MSG_LEN];
} sys_msg_t;

void ui_window_init();
void ui_msg_init();
const char* ui_chan_get_cur();

/* normal UI funcition, called in main loop; */
int ui_msg_send(const char *chan, const char *msg, const char *img);
int ui_chan_add(const char *chan);
int ui_chan_rm(const char *chan);

/* These function oftenly called as a idle, so thire type muse be:
 *      gboolean (*GSourceFunc) (gpointer user_data);
 * they accept a sturcture pointer which point to the parameters they required
 * this structure should be free at the end of function
 */
gboolean ui_chan_set_topic(topic_t *topic_t);
gboolean ui_online_list_add(online_list_item_t *item);
gboolean ui_online_list_rm(online_list_item_t *item);
gboolean ui_msg_recv(recv_msg_t *msg);
gboolean ui_msg_sys(sys_msg_t *msg);

#endif /* __UI_H */
