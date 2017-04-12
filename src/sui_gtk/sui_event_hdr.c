// #define __DBG_ON
#define __LOG_ON

#include "sui_event_hdr.h"

#include "srain.h"
#include "log.h"

/* extern from sui.c */
extern SuiAppEvents *app_events;

void sui_event_hdr(SuiSession *sui, SuiEvent event, const char *params[], int count){
    SuiEvents *events;

    /* Debug output and parameters check */
    bool nullparam = FALSE;
    DBG_FR("sui: %p, event: %d", sui, event);
    for (int i = 0; i < count; i++){
        if (i == 0) DBG_F("count: %d, params: [", count);
        if (i == count - 1) {
            DBG("%s]\n", params[i]);
        } else {
            DBG("%s ", params[i]);
        }
        if (!params[i]) nullparam = TRUE;
    }
    g_return_if_fail(!nullparam);

    if (sui){
        events = sui_get_events(sui);
        g_return_if_fail(events);
    } else {
        events = NULL;
    }

    switch (event) {
        /* App events */

        case SUI_EVENT_ACTIVATE:
            g_return_if_fail(app_events->activate);
            app_events->activate(event, params, count);
            break;
        case SUI_EVENT_CONNECT:
            g_return_if_fail(app_events->connect);
            app_events->connect(event, params, count);
            break;

        /* Events */
        case SUI_EVENT_DISCONNECT:
            g_return_if_fail(events->disconnect);
            events->disconnect(sui, event, params, count);
            break;
        case SUI_EVENT_SEND:
            g_return_if_fail(events->send);
            events->send(sui, event, params, count);
            break;
        case SUI_EVENT_JOIN:
            g_return_if_fail(events->join);
            events->join(sui, event, params, count);
            break;
        case SUI_EVENT_PART:
            g_return_if_fail(events->part);
            events->part(sui, event, params, count);
            break;
        case SUI_EVENT_QUERY:
            g_return_if_fail(events->query);
            events->query(sui, event, params, count);
            break;
        case SUI_EVENT_UNQUERY:
            g_return_if_fail(events->unquery);
            events->unquery(sui, event, params, count);
            break;
        case SUI_EVENT_KICK:
            g_return_if_fail(events->kick);
            events->kick(sui, event, params, count);
            break;
        case SUI_EVENT_INVITE:
            g_return_if_fail(events->invite);
            events->invite(sui, event, params, count);
            break;
        case SUI_EVENT_WHOIS:
            g_return_if_fail(events->whois);
            events->whois(sui, event, params, count);
            break;
        case SUI_EVENT_IGNORE:
            g_return_if_fail(events->ignore);
            events->ignore(sui, event, params, count);
            break;
        case SUI_EVENT_CUTOVER:
            g_return_if_fail(events->cutover);
            events->cutover(sui, event, params, count);
            break;
        default:
            ERR_FR("No such SuiEvent: %d", event);
    }
}
