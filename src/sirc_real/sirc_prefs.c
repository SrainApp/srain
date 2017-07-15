#include <glib.h>

#include "sirc/sirc.h"
#include "i18n.h"

SircPrefs *sirc_prefs_new(){
    SircPrefs *prefs;

    prefs = g_malloc0(sizeof(SircPrefs));

    return prefs;
}

SrnRet sirc_prefs_is_valid(SircPrefs *prefs){
    if (!prefs){
        return RET_ERR(_("Invalid ServerPrefs instance"));
    }
    return SRN_OK;
}

void sirc_prefs_free(SircPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs);
}
