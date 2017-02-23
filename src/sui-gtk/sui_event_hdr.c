#define __LOG_ON

#include "sui_event_hdr.h"

#include "log.h"

/* extern from sui.c */
extern SuiAppEvents *app_events;

void sui_event_hdr(SuiSession *sui, SuiEvent event, const char *params[], int count){
    SuiEvents *events;

    LOG_FR("sui: %p, event: %d", sui, event);

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
    }
}
