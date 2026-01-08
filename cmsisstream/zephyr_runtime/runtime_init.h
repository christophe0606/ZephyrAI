#ifndef stream_runtime_init_h
#define stream_runtime_init_h 

#include <zephyr/kernel.h>

#ifdef   __cplusplus
extern "C"
{
#endif 


extern struct k_mem_slab cg_eventPool;
extern struct k_mem_slab cg_bufPool;
extern struct k_mem_slab cg_mutexPool;

typedef uint32_t (*stream_scheduler)(int *error,void *graphData);
/**
 * @brief Initialize the CMSIS Stream runtime memory and structures
 * @return 0 on success, negative error code on failure
 * 
 * This function initializes the memory pools and event queue required
 * for the CMSIS Stream runtime in a Zephyr environment.
 */
extern int init_stream_memory();

/**
 * @brief Start the CMSIS Stream runtime threads
 * This function creates and starts the necessary threads for
 * handling CMSIS Stream events and processing.
 */
extern void start_stream_threads(stream_scheduler scheduler,void *params);

/**
 * @brief Wait for the CMSIS Stream threads to finish
 * This function blocks until the stream and event threads
 * have completed their execution.
 */
extern void wait_for_stream_thread_end();

/**
 * @brief Free the CMSIS Stream runtime memory
 * This function releases the memory allocated for the CMSIS Stream
 * runtime, including the event queue.
 */
extern void free_stream_memory();

/**
 * @brief Create a new CMSIS Stream Event Queue
 * @return Pointer to the newly created Event Queue, or nullptr on failure
 * This function also set this event queue as the current queue for the
 * event thread. This function should be called only when the event thread
 * is not running.
 */
extern void *new_event_queue();




#ifdef   __cplusplus
}
#endif

#endif