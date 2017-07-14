#ifndef __SUI_PREFS_H
#define __SUI_PREFS_H

#ifndef __IN_SUI_H
	#error This file should not be included directly, include just sui.h
#endif

#include "srain.h"
#include "ret.h"

typedef struct _SuiAppPrefs SuiAppPrefs;
typedef struct _SuiPrefs SuiPrefs;

struct _SuiAppPrefs {
    char *theme;
    // const char *font;
};

struct _SuiPrefs {
    bool notify;
    bool show_topic;
    bool show_avatar;
    bool show_user_list;
    bool preview_image;
};

SuiAppPrefs *sui_app_prefs_new();
SrnRet sui_app_prefs_is_valid(SuiAppPrefs *prefs);
void sui_app_prefs_free(SuiAppPrefs *prefs);

SuiPrefs *sui_prefs_new();
SrnRet sui_prefs_is_valid(SuiPrefs *prefs);
void sui_prefs_free(SuiPrefs *prefs);

#endif /* __SUI_PREFS_H */
