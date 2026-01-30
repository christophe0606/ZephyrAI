#if defined(CONFIG_MODEL_IN_EXT_FLASH)
#include <zephyr/kernel.h>

extern "C" {
#include "network.h"
#include "md5.h"
}

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>

#define OSPI1_NODE DT_NODELABEL(ospi1)

#if !DT_NODE_HAS_STATUS(OSPI1_NODE, okay)
#error "ospi1 node is not defined or not okay"
#endif

#define XIP_ADDR_HE DT_PROP_BY_IDX(OSPI1_NODE, xip_base_address, 0)

const nn_header_t *nn_header=reinterpret_cast<const nn_header_t *>(reinterpret_cast<const uint8_t*>(XIP_ADDR_HE));


const nn_header_t *get_network_description()
{
    return nn_header;
}

static uint32_t get_description_length()
{
    return nn_header->size;
}

int validate_network_description(const char *expected_md5_hex)
{
    unsigned char md5_sum[16];
    char md5_hex[33];

    const void *data = get_network_description();

    size_t data_len = get_description_length();

    md5_compute(data, data_len, md5_sum);
    md5_to_hex(md5_sum, md5_hex);

    if (std::strncmp(md5_hex, expected_md5_hex, 32) != 0)
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

namespace arm {
namespace app {
namespace kws {

const uint8_t * GetModelPointer()
{
#if defined(DEBUG_MODE)
        return reinterpret_cast<const uint8_t *>(XIP_ADDR_HE);
#else
    return nn_header->models[0].nn_model_data;
#endif
}

size_t GetModelLen()
{
#if defined(DEBUG_MODE)
    return 0x00026570;
#else
    return nn_header->models[0].nn_model_size;
#endif
}

} /* namespace arm */
} /* namespace app */
} 
#endif