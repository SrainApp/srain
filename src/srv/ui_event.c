#define __DBG_ON
#define __LOG_ON

#include "ui_event.h"

#include "srain.h"
#include "log.h"

#define PRINT_EVENT_PARAM \
    do { \
        DBG_FR("sui: 0x%p, event: %d", sui, event); \
        for (int i = 0; i < count; i++){ \
            if (i == 0) DBG_F("count: %d, params: [", count); \
            if (i == count - 1) { \
                DBG("%s]\n", params[i]); \
            } else { \
                DBG("%s ", params[i]); \
            } \
        } \
    } while (0)

int ui_event_connect(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_send(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_join(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_part(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_query(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_unquery(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_kick(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
int ui_event_invite(SuiSession *sui, SuiEvent event, const char *params[], int count){
    PRINT_EVENT_PARAM;
    return SRN_OK;
}
