#ifndef __UI_EVENT_H
#define __UI_EVENT_H

#include "sui/sui.h"

int ui_event_connect(SuiSession *sui, SuiEvent event, const char *params[], int count);
int ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count);
int ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count);
int ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count);
int ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count);
int ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count);
int ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count);
int ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count);

#endif /* __UI_EVENT_H */
