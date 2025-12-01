#ifndef config_h
#define config_h



#define DEBUG_PRINT(fmt, ...) \
    LOG_DBG(fmt, ##__VA_ARGS__)
 

#define ERROR_PRINT(fmt, ...) \
    LOG_ERR(fmt, ##__VA_ARGS__)


#endif