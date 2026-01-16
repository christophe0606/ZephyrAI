#pragma once 

#include <zephyr/kernel.h>

#if defined(CONFIG_I2S)
extern const struct device *init_audio_source(k_mem_slab **mem_slab_out);
#endif 

#if defined(CONFIG_DISPLAY)
extern int init_display();
extern void clear_display();
#endif