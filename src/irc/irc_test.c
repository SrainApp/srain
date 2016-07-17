#define __LOG_ON

#include <assert.h>

#include "irc.h"
#include "log.h"
#include "meta.h"

void irc_test(){
    irc_init();
    assert(irc_session_new("127.0.0.1", 6667, "srainbot", PACKAGE_VERSION, PACKAGE_WEBSITE) == 0);
    assert(irc_session_new("127.0.0.1", 6667, "srainbot2", PACKAGE_VERSION, PACKAGE_WEBSITE) == 0);
    while (1) irc_session_process();
}
