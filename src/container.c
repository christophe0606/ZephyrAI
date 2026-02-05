#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "container.h"
#include "md5.h"

#include <string.h>
#include <stdio.h>

LOG_MODULE_DECLARE(streamapps, CONFIG_STREAMAPPS_LOG_LEVEL);

#if defined(CONFIG_MODEL_IN_EXT_FLASH)

#define OSPI1_NODE DT_NODELABEL(ospi1)

#if !DT_NODE_HAS_STATUS(OSPI1_NODE, okay)
#error "ospi1 node is not defined or not okay"
#endif

#define XIP_ADDR_HE DT_PROP_BY_IDX(OSPI1_NODE, xip_base_address, 0)

// Datatypes to save several binaries in flash
// Binary description header
typedef struct {
    uint32_t binary_size;
    const uint8_t *binary_data;
} binary_desc_t;

// List of binaries
struct container_header_t_ {
    uint32_t size;
    uint32_t magic;
    uint32_t nb_binaries;
    binary_desc_t binaries[];
};

static const container_header_t *container_header=(const container_header_t *)(XIP_ADDR_HE);


const container_header_t *get_container_description()
{
    return (container_header);
}

const uint8_t * get_binary(uint32_t idx)
{
   return (container_header->binaries[idx].binary_data);
}

uint32_t get_binary_len(uint32_t idx)
{
   return (container_header->binaries[idx].binary_size);
}

static uint32_t get_container_length()
{
    return (container_header->size);
}

int validate_container_description(const char *expected_md5_hex)
{
    unsigned char md5_sum[16];
    char md5_hex[33];

    const container_header_t *header = get_container_description();
    const void *data = (const void *)(header);

    if (header->magic != 0xBEEFDEAD)
    {
        LOG_ERR("Invalid container description header magic: expected 0xBEEFDEAD, got 0x%08X\n", header->magic);
        return 1; // Invalid header
    }

    size_t data_len = get_container_length();

    md5_compute(data, data_len, md5_sum);
    md5_to_hex(md5_sum, md5_hex);

    if (strncmp(md5_hex, expected_md5_hex, 32) != 0)
    {
        for(int i = 0; i < 32; i++)
        {
            printf("%c", md5_hex[i]);
        }
        printf("\n");

        for(int i = 0; i < 32; i++)
        {
            printf("%c", expected_md5_hex[i]);
        }
        printf("\n");
        return 1; // checksum does not match
    }

    return 0; // Valid description
}

#endif
