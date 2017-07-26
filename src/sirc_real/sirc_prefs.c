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

char* sirc_prefs_dump(SircPrefs *prefs){
    GString *str;
    g_return_val_if_fail(prefs, NULL);

    const char *t = _("True");
    const char *f = _("False");

    str = g_string_new("");
    g_string_append_printf(str,
            _("TLS: %s, TLS verify certificate: %s"),
            prefs->tls ? t : f, prefs->tls_not_verify ? f : t);

    char *dump = str->str;
    g_string_free(str, FALSE);

    return dump;
}

void sirc_prefs_free(SircPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs);
}
