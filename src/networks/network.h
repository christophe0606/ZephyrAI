#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <stddef.h>

// When defined, simple mode enabled. The raw model (without header) is stored in flash
//#define DEBUG_MODE

// Datatypes to save several models in flash
// Model description header
typedef struct {
    uint32_t nn_model_size;
    uint8_t *nn_model_data;
} nn_model_t;

// List of models
typedef struct {
    uint32_t size;
    uint32_t magic;
    uint32_t nb_models;
    nn_model_t models[];
} nn_header_t;

extern const nn_header_t *nn_header;

// If CONFIG_MODEL_IN_EXT_FLASH get the first model in flash or the simple model in case of DEBUG_MODE
// If flash mode not enabled, get the .cpp model that has been compiled in the binary.
const uint8_t * GetModelPointer();
size_t GetModelLen();

#if defined(CONFIG_MODEL_IN_EXT_FLASH)
// Get the network description header from external flash
nn_header_t *get_network_description();
// Compute a MD5 checksum of the network description and compare it to the expected one
// It includes the header and all the models
int validate_network_description(const char *expected_md5_hex);
#endif

#endif