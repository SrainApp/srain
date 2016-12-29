/**
 * @file ui_hdr.c
 * @brief IRC server interface for UI module
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
 * @date 2016-04-21
 */
#include "ui_hdr.h"
#include "ui_test.h"

#include "log.h"

void ui_hdr_init(){
#ifdef UI_TEST
    ui_hdr_srv_join = ui_test_srv_join;
    ui_hdr_srv_part = ui_test_srv_part;
    ui_hdr_srv_send = ui_test_srv_send;
    ui_hdr_srv_cmd = ui_test_srv_cmd;
    // TODO: more...
#else
    ui_hdr_srv_connect = srv_connect;
    ui_hdr_srv_query = srv_query;
    ui_hdr_srv_unquery = srv_unquery;
    ui_hdr_srv_join = srv_join;
    ui_hdr_srv_part = srv_part;
    ui_hdr_srv_quit = srv_quit;
    ui_hdr_srv_send = srv_send;
    ui_hdr_srv_cmd = srv_cmd;
    ui_hdr_srv_kick = srv_kick;
    ui_hdr_srv_whois = srv_whois;
    ui_hdr_srv_invite= srv_invite;
#endif
}
