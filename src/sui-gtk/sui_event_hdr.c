#define __LOG_ON

#include "sui_event_hdr.h"

#include "log.h"

void sui_event_hdr(SuiSession *sui, SuiEvent event, const char *params[], int count){
    SuiEvents *events;

    LOG_FR("sui: %p, event: %d", sui, event);
    events = sui_get_events(sui);
    g_return_if_fail(events);

    switch (event) {
        case SUI_EVENT_CONNECT:
            g_return_if_fail(events->connect);
            events->connect(sui, event, params, count);
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
