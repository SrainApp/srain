#ifndef __PREFS_H
#define __PREFS_H

#include "srain.h"
#include "server.h"
#include "log.h"
#include "ret.h"

void prefs_init();
SrnRet prefs_read();
void prefs_finalize();

SrnRet prefs_read_log_prefs(LogPrefs *prefs);
SrnRet prefs_read_sui_app_prefs(SuiAppPrefs *prefs);
SrnRet prefs_read_server_prefs_list();
SrnRet prefs_read_server_prefs(ServerPrefs *prefs);
SrnRet prefs_read_sirc_prefs(SircPrefs *prefs, const char *srv_name);
SrnRet prefs_read_sui_prefs(SuiPrefs *prefs, const char *srv_name, const char *chat_name);

#endif /*__PREFS_H */
