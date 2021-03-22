#ifndef __SIRC_MESSENGER_H
#define __SIRC_MESSENGER_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SIRC_TYPE_MESSENGER (sirc_messenger_get_type())

G_DECLARE_FINAL_TYPE(SircMessenger, sirc_messenger, SIRC, MESSENGER, GObject)

SircMessenger *sirc_messenger_new(void);

G_END_DECLS

#endif /* __SIRC_MESSENGER_H */
