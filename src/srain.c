/**
 * @file main.c
 * @brief main function here
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */


#include <stdlib.h>
#include <signal.h>

#include "server.h"

#include "filter.h"
#include "decorator.h"

#include "ret.h"
#include "i18n.h"
#include "prefs.h"
#include "plugin.h"
#include "file_helper.h"

static void quit();

int main(int argc, char **argv){
    signal(SIGINT, quit);

    ret_init();
    log_init();
    i18n_init();
    prefs_init();

    create_user_file();

    plugin_init();

    filter_init();
    decorator_init();

    server_init();

    quit();

    return 0;
}

static void quit(){
    prefs_finalize();
    plugin_finalize();
    server_finalize();
    log_finalize();
    ret_finalize();

    exit(0);
}
