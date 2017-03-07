/**
 * @file main.c
 * @brief main function here
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <stdlib.h>
#include <signal.h>

#include "server.h"

#include "decorator.h"

#include "i18n.h"
#include "plugin.h"
#include "file_helper.h"

static void quit();

int main(int argc, char **argv){
    create_user_file();
    signal(SIGINT, quit);

    i18n_init();
    plugin_init();

    decorator_init();

    server_init();

    quit();

    return 0;
}

static void quit(){
    plugin_finalize();
    server_finalize();

    exit(0);
}
