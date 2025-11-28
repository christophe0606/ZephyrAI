#ifndef config_h
#define config_h



#define NB_BINS 128

#define SAMPLE_RATE 16000

#include <stdio.h>
#include <stdint.h>

#if 0
#define DEBUG_PRINT(fmt, ...) \
    fprintf(stderr, "[DEBUG] " fmt, ##__VA_ARGS__)
#else 

#define DEBUG_PRINT(fmt, ...)
#endif 

#if 1
#define ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] " fmt, ##__VA_ARGS__)
#else
#define ERROR_PRINT(fmt, ...)
#endif 


#endif