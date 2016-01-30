#ifndef __I18N_H
#define __I18N_H

#include <libintl.h>
#include <locale.h>

#define _(str) gettext(str)
void i18n_init(void);

#endif /* __I18N_H */
