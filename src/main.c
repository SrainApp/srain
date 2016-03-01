/**
 * @file main.c
 * @brief main function here
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include "srain_app.h"

int main(int argc, char **argv){
    return g_application_run(G_APPLICATION(srain_app_new()), argc, argv);
}
