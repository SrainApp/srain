#include "core/core.h"

SrnApplicationConfig *srn_application_config_new(void){
    SrnApplicationConfig *cfg;

    cfg = g_malloc0(sizeof(SrnApplicationConfig));
    cfg->ui = sui_application_config_new();

    return cfg;
}

void srn_application_config_free(SrnApplicationConfig *cfg){
    g_list_free_full(cfg->auto_connect_srv_list, g_free);
    sui_application_config_free(cfg->ui);
    g_free(cfg);
}

SrnRet srn_application_config_check(SrnApplicationConfig *cfg){
    return SRN_OK;
}
