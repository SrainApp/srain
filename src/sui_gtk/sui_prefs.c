SuiAppPrefs *sui_app_prefs_new(){
    SuiAppPrefs *prefs;

    prefs = g_malloc0(sizeof(SuiAppPrefs));

    return prefs;
}

bool sui_app_prefs_is_valid(SuiAppPrefs *prefs){
    return (prefs
            && prefs->theme);
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

bool sui_app_prefs_is_valid(SuiPrefs *prefs){
    return prefs;
}

void sui_prefs_free(SuiPrefs *prefs){
    g_return_if_fail(prefs);

    g_free(prefs);
}
