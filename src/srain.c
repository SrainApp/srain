#include <stdio.h>
#include "i18n.h"

int main(){
    setlocale(LC_ALL, "");
    textdomain("srain");
    bindtextdomain("srain", "locale");
    printf(_("Hello srain!\n"));
    return 0;
}
