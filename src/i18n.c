#define __LOG_ON

#include "i18n.h"
#include "log.h"

void i18n_init(){
    // Set the current local to default
    setlocale(LC_ALL, "");

    /* Specify that the DOMAINNAME message catalog
     * will be found in DIRNAME rather than in
     * the system locale data base
     */
    bindtextdomain("srain", "locale");

    // Set the current default message catalog to DOMAINNAME.
    textdomain("srain");

    LOG_FR(_("Language: English"));
}
