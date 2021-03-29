#ifndef __SIRC_LOGIN_FLOW_H
#define __SIRC_LOGIN_FLOW_H

#include <glib-object.h>

G_BEGIN_DECLS

#define SIRC_TYPE_LOGIN_FLOW sirc_login_flow_get_type()

G_DECLARE_FINAL_TYPE(SircLoginFlow, sirc_login_flow, SIRC, LOGIN_FLOW, GObject)

SircLoginFlow *sirc_login_flow_new(void);

G_END_DECLS

#endif /* __SIRC_LOGIN_FLOW_H */
