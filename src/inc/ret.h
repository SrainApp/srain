#ifndef __RET_H
#define __RET_H

/* Reserved return id */
#define SRN_OK      0   // No error occurred
#define SRN_ERR    -1   // Unknown/Unspecified error

/* Just alias */
#define RET_OK(...) ret_ok(__VA_ARGS__)
#define RET_ERR(...) ret_err(__VA_ARGS__)
#define RET_IS_OK(id) ({ typeof (id) _id = (id); \
        (_id == SRN_OK || ret_get_no(_id) == SRN_OK);})
#define RET_IS_ERR(id) ({ typeof (id) _id = (id); \
        (_id == SRN_ERR || ret_get_no(_id) == SRN_ERR);})
#define RET_MSG(id) ret_get_message((id))

typedef int SrnRet;

void ret_init();
void ret_finalize();
SrnRet ret_err(const char *fmt, ...);
SrnRet ret_ok(const char *fmt, ...);
const char *ret_get_message(SrnRet id);
int ret_get_no(SrnRet id);

#endif /* __RET_H */
