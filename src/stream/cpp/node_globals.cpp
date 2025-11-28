#include "node_globals.hpp"
#include "custom.hpp"
#include "cstream_node.h"
#include "scheduler.h"

// Audio output node global state
// Used to communicate between SAI IRQ and audio output node
audioOutputGlobalState_t g_audioOutputState;

uint8_t tensorArena[ACTIVATION_BUF_SZ] ACTIVATION_BUF_ATTRIBUTE;
