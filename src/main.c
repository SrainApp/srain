/**
 * @file main.c
 * @brief main function here
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <stdlib.h>
#include "srain_app.h"
#include "log.h"

#ifndef DESTDIR
#define DESTDIR "NULL"
#endif

int main(int argc, char **argv){
    LOG_FR("DESTDIR = %s", DESTDIR);
    LOG_FR("change cwd to" DESTDIR  "/share/srain");

    chdir(DESTDIR  "/share/srain");
    return g_application_run(G_APPLICATION(srain_app_new()), argc, argv);
}
