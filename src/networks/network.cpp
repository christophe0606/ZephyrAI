extern "C" {
#include "network.h"
#include "container.h"
}

#include <zephyr/kernel.h>

namespace arm {
namespace app {
namespace kws {

extern const uint8_t *GetModelPointer();
extern size_t GetModelLen();

}  // namespace kws
}  // namespace app
}  

#if defined(CONFIG_MODEL_IN_EXT_FLASH) && defined(USE_FLASH)

#if defined(DEBUG_MODE)

#define OSPI1_NODE DT_NODELABEL(ospi1)

#if !DT_NODE_HAS_STATUS(OSPI1_NODE, okay)
#error "ospi1 node is not defined or not okay"
#endif

#define XIP_ADDR_HE DT_PROP_BY_IDX(OSPI1_NODE, xip_base_address, 0)

#endif /* DEBUG MODE */

namespace arm {
namespace app {
namespace kws {

const uint8_t * GetModelPointer()
{
#if defined(DEBUG_MODE)
        return reinterpret_cast<const uint8_t *>(XIP_ADDR_HE);
#else
    return get_binary(0);
#endif
}

size_t GetModelLen()
{
#if defined(DEBUG_MODE)
    return 0x00026570;
#else
    return get_binary_len(0);
#endif
}

} /* namespace arm */
} /* namespace app */
} 
#endif /* Use flash */


const uint8_t * GetModelPointer()
{
   return arm::app::kws::GetModelPointer();
}

size_t GetModelLen()
{
   return arm::app::kws::GetModelLen();
}

