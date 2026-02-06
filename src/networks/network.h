#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <stddef.h>

// When defined, simple mode enabled. The raw model (without header) is stored in flash
//#define DEBUG_MODE
// When flash enabled in device tree, this flag must ALSO
// be enabled to use the network from flash.
#define USE_FLASH


// If CONFIG_MODEL_IN_EXT_FLASH get the first model in flash or the simple model in case of DEBUG_MODE
// If flash mode not enabled, get the .cpp model that has been compiled in the binary.
extern const uint8_t * GetModelPointer();
extern size_t GetModelLen();


#endif