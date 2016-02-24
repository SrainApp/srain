#include <gtk/gtk.h>
#include "srain_app.h"

int main(int argc, char **argv){
    return g_application_run(G_APPLICATION(srain_app_new()), argc, argv);
}
