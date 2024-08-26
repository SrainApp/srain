#include "core/core.h"
#include "i18n.h"

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
    if (g_strcmp0(cfg->ui->window.chat_list_order, CHAT_LIST_ORDER_RECENT) != 0
            && g_strcmp0(cfg->ui->window.chat_list_order, CHAT_LIST_ORDER_ALPHABET) != 0){
        return RET_ERR(_("Invalid chat-list-order configuration"));
    }
    return SRN_OK;
}
