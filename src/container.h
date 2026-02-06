#ifndef container_h
#define container_h

#include <stdint.h>
#include <stddef.h>


#if defined(CONFIG_MODEL_IN_EXT_FLASH)

struct container_header_t_;
typedef struct container_header_t_ container_header_t;

// Get the container description header from external flash
extern const container_header_t *get_container_description();
// Compute a MD5 checksum of the container description and compare it to 
// the expected one.
// It includes the header and all the binaries
extern int validate_container_description(const char *expected_md5_hex);

extern const uint8_t * get_binary(uint32_t idx);
extern uint32_t get_binary_len(uint32_t idx);
#endif

#endif
