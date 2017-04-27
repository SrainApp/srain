#ifndef __SUI_MESSAGE_H
#define __SUI_MESSAGE_H

#include <gtk/gtk.h>

#define SUI_MESSAGE         \
    GtkBox parent;          \
    GtkLabel *msg_label;    \
    GtkLabel *time_label;   \
    GtkBox *padding_box;    \
    void *ctx;              \

struct _SuiMessage {
    SUI_MESSAGE;
};

#endif /* __SUI_MESSAGE_H */
