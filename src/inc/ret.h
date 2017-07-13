#ifndef __RET_H
#define __RET_H

/* Just alias */
#define ERR(...) ret_err(__VA_ARGS__)
#define ERRMSG(id) ret_errmsg((id))

/* Reserved error id */
#define SRN_OK      0   // No error occurred
#define SRN_ERR    -1   // Unknown/Unspecified error

typedef int SrnRet;

void ret_init();
void ret_finalize();
SrnRet ret_err(const char *fmt, ...);
const char *ret_errmsg(int id);

#endif /* __RET_H */
