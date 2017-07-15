#ifndef __SIRC_PREFS_H
#define __SIRC_PREFS_H

#ifndef __IN_SIRC_H
	#error This file should not be included directly, include just sirc.h
#endif

#include "srain.h"
#include "ret.h"

typedef struct _SircPrefs SircPrefs;

struct _SircPrefs {
    bool auto_reconnect;
    bool use_ssl;
    bool verify_ssl_cert;
    // bool use_ipv6;
    // bool use_sasl;
};

SircPrefs *sirc_prefs_new();
SrnRet sirc_prefs_is_valid(SircPrefs *prefs);
void sirc_prefs_free(SircPrefs *prefs);

#endif /* __SIRC_PREFS_H */
