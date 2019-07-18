/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <strings.h>
#include <glib.h>

#include "sirc/sirc.h"
#include "srain.h"
#include "log.h"

/* RFC 2812 https://tools.ietf.org/html/rfc2812#section-2.3
 *
 * servername = hostname
 * hostname = shortname *( "." shortname )
 * shortname = ( letter / digit ) *( letter / digit / "-" ) *( letter / digit )
 * nickname = ( letter / special ) *8( letter / digit / special / "-" )
 * channel =  ( "#" / "+" / ( "!" channelid ) / "&" ) chanstring [ ":" chanstring ]
 * channelid  = 5( A-Z / 0-9 )
 * chanstring =  %x01-07 / %x08-09 / %x0B-0C / %x0E-1F / %x21-2B
 * chanstring =/ %x2D-39 / %x3B-FF
 * letter = A-Z / a-z
 * digit = 0-9
 * special = "[", "]", "\", "`", "_", "^", "{", "|", "}"
 */

#define SIRC_LATTER_PATTERN     "\\pL"
#define SIRC_DIGIT_PATTERN      "\\pN"
#define SIRC_SPECIAL_PATTERN    "[\\[\\]\\\\`_^{|}]"
#define SIRC_NICKNAME_PATTERN   "\\A" "(" SIRC_LATTER_PATTERN "|" SIRC_DIGIT_PATTERN ")" \
                                "(" SIRC_LATTER_PATTERN "|" SIRC_DIGIT_PATTERN "|" SIRC_SPECIAL_PATTERN "|" "-" ")" "*" "\\Z"
#define SIRC_SHORTNAME_PATTERN  "(" SIRC_LATTER_PATTERN "|" SIRC_DIGIT_PATTERN ")" \
                                "(" SIRC_LATTER_PATTERN "|" SIRC_DIGIT_PATTERN "|" "-" ")" "*" \
                                "(" SIRC_LATTER_PATTERN "|" SIRC_DIGIT_PATTERN ")" "*"
#define SIRC_HOSTNAME_PATTERN   "\\A" "(" SIRC_SHORTNAME_PATTERN ")" "(" "\\." SIRC_SHORTNAME_PATTERN ")" "+"  "\\Z"
#define SIRC_SERVERNAME_PATTERN SIRC_HOSTNAME_PATTERN
#define SIRC_CHANSTRING_PATTERN "[^\\r\\n:,;\\s]"
#define SIRC_CHANNELID_PATTERN  "(" SIRC_LATTER_PATTERN "|" SIRC_DIGIT_PATTERN ")" "{5}"
#define SIRC_CHANNEL_PATTERN    "\\A" "(" "[#+&]" "|" "!" SIRC_CHANNELID_PATTERN ")" SIRC_CHANSTRING_PATTERN "*" \
                                "(" ":" SIRC_CHANSTRING_PATTERN ")" "*" "\\Z"

bool sirc_target_equal(const char *target1, const char *target2){
    return g_ascii_strcasecmp(target1, target2) == 0;
}

// TODO: Test for sirc_target_is_XXX

bool sirc_target_is_servername(SircSession *sirc, const char *target){
    static GRegex *regex;

    if (!regex){
        const char *pattern;
        GError *err;

        pattern = SIRC_SERVERNAME_PATTERN;
        err = NULL;
        regex = g_regex_new(pattern, G_REGEX_CASELESS | G_REGEX_OPTIMIZE, 0, &err);
        if (err){
            ERR_FR("g_regex_new() failed, pattern: %s, err: %s",
                    pattern, err->message);
            g_error_free(err);
            return FALSE;
        }
    }
    return g_regex_match(regex, target, 0, NULL);
}

bool sirc_target_is_nickname(SircSession *sirc, const char *target){
    // TODO: Nick length
    static GRegex *regex;

    if (!regex){
        const char *pattern;
        GError *err;

        pattern = SIRC_NICKNAME_PATTERN;
        err = NULL;
        regex = g_regex_new(pattern, G_REGEX_CASELESS | G_REGEX_OPTIMIZE, 0, &err);
        if (err){
            ERR_FR("g_regex_new() failed, pattern: %s, err: %s",
                    pattern, err->message);
            return FALSE;
        }
    }
    return g_regex_match(regex, target, 0, NULL);
}

bool sirc_target_is_service(SircSession *sirc, const char *target){
    if (!sirc_target_is_nickname(sirc, target)){
        return FALSE;
    }
    return g_str_has_suffix(target, "Serv") || g_str_has_suffix(target, "serv");
}

bool sirc_target_is_channel(SircSession *sirc, const char *target){
    // TODO: Channel length
    static GRegex *regex;

    if (!regex){
        const char *pattern;
        GError *err;

        pattern = SIRC_CHANNEL_PATTERN;
        err = NULL;
        regex = g_regex_new(pattern, G_REGEX_CASELESS | G_REGEX_OPTIMIZE, 0, &err);
        if (err){
            ERR_FR("g_regex_new() failed, pattern: %s, err: %s",
                    pattern, err->message);
            g_error_free(err);
            return FALSE;
        }
    }
    return g_regex_match(regex, target, 0, NULL);
}

/* TODO */
const char* sirc_prefix_get_target(const char *prefix){
    return NULL;
}

const char* sirc_prefix_get_host(const char *prefix){
    return NULL;
}

const char* sirc_prefix_get_user(const char *prefix){
    return NULL;
}
