#ifndef __LOG_H
#define __LOG_H

#define LOG(...) \
    g_print("%d: [%s]: ", __LINE__, __FUNCTION__);   \
    g_print(__VA_ARGS__)

#endif /* __LOG_H */
