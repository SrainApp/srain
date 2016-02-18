#ifndef __UI_H
#define __UI_H

#include <glib.h>
#include <irc.h>

#define TIME_LEN 32
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

/* ui_msg_send use fields: chan, msg, time, img
 * ui_msg_sys use fields: chan, msg, locked
 * ui_msg_recv use fields: all
 */
typedef struct {
    char id[NICK_LEN];
    char nick[NICK_LEN];
    char chan[CHAN_LEN];
    char msg[MSG_LEN];
    char time[TIME_LEN];
    char avatar[PATH_LEN];   // path of cached avatar, can be null
    char img[PATH_LEN];      // path of cached img, can be null
} bubble_msg_t;

void ui_window_init();
void ui_msg_init();
int ui_msg_send(bubble_msg_t *msg);
const char* ui_chan_get_cur();

/* These function oftenly called as a idle, so thire type muse be:
 *      gboolean (*GSourceFunc) (gpointer user_data);
 * they accept a sturcture pointer which point to the parameters they required
 * this structure should be free at the end of function
 */
gboolean ui_chan_add(chan_name_t *chan);
gboolean ui_chan_rm(chan_name_t *chan);
gboolean ui_chan_set_topic(topic_t *topic_t);
gboolean ui_online_list_add(online_list_item_t *item);
gboolean ui_online_list_rm(online_list_item_t *item);
gboolean ui_msg_recv(bubble_msg_t *msg);
gboolean ui_msg_sys(bubble_msg_t *msg);

#endif /* __UI_H */
