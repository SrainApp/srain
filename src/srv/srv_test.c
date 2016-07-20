#define __LOG_ON

#include <assert.h>

#include "srv.h"
#include "log.h"
#include "meta.h"

void srv_test(){
    srv_init();
    assert(srv_connect("127.0.0.1", 6667, NULL, "la", PACKAGE_VERSION, PACKAGE_WEBSITE) == 0);
    // assert(srv_connect("localhost", 6667, NULL, "srainbot2", PACKAGE_VERSION, PACKAGE_WEBSITE) == 0);

    while (1);
}
