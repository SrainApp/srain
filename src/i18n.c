#include "i18n.h"

void i18n_init(){
    setlocale(LC_ALL, "");
    textdomain("srain");
    bindtextdomain("srain", "locale");
}
