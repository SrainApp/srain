#include <glib.h>

#include "sirc/sirc.h"

SircPrefs *sirc_prefs_new(){
    SircPrefs *prefs; 

    prefs = g_malloc0(sizeof(SircPrefs));

    return prefs;
}

bool sirc_prefs_is_valid(SircPrefs *prefs){
    return prefs;
}

void sirc_prefs_free(SircPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs);
}
