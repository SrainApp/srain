#include <glib.h>

#include "sui/sui.h"
#include "i18n.h"

SuiAppPrefs *sui_app_prefs_new(){
    SuiAppPrefs *prefs;

    prefs = g_malloc0(sizeof(SuiAppPrefs));

    return prefs;
}

SrnRet sui_app_prefs_is_valid(SuiAppPrefs *prefs){
    const char *fmt = _("Missing field in SuiAppPrefs: %s");

    if (!prefs){
        return ERR(_("Invalid SuiAppPrefs instance"));
    }
    if (!prefs->theme){
        return ERR(fmt, "theme");
    }
    return SRN_OK;
}

void sui_app_prefs_free(SuiAppPrefs *prefs){
    g_return_if_fail(prefs);

    if (prefs->theme){
        g_free(prefs->theme);
        prefs->theme = NULL;
    }

    g_free(prefs);
}

SuiPrefs *sui_prefs_new(){
    SuiPrefs *prefs;

    prefs = g_malloc0(sizeof(SuiPrefs));

    return prefs;
}

SrnRet sui_prefs_is_valid(SuiPrefs *prefs){
    if (!prefs){
        return ERR(_("Invalid SuiPrefs instance"));
    }
    return SRN_OK;
}

void sui_prefs_free(SuiPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs);
}
