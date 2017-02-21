#ifndef __SUI_EVENT_H
#define __SUI_EVENT_H

#ifndef __IN_SUI_H
	#error This file should not be included directly, include just sui.h
#endif

typedef enum {
    SUI_EVENT_CONNECT,
    SUI_EVENT_SEND,
    SUI_EVENT_JOIN,
    SUI_EVENT_PART,
    SUI_EVENT_QUERY,
    SUI_EVENT_UNQUERY,
    SUI_EVENT_KICK,
    SUI_EVENT_INVITE,
} SuiEvent;

typedef int (*SuiEventCallback) (SuiSession *sui, SuiEvent event,
        const char *params[], int count);

typedef struct {
    SuiEventCallback connect;
    SuiEventCallback send;
    SuiEventCallback join;
    SuiEventCallback part;
    SuiEventCallback query;
    SuiEventCallback unquery;
    SuiEventCallback kick;
    SuiEventCallback invite;
} SuiEvents;

void sui_event_hdr(SuiSession *sui, SuiEvent event, const char *params[], int count);

#endif /* __SUI_EVENT_H */
