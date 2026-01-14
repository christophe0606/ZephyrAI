#pragma once 

/**
 * 
 * This file is used to customize the dataflow loop and provide
 * some definition  to the scheduler.cpp
 * Generally this file defines some macros used in the
 * scheduler dataflow loop and some datatypes used in the project.
 * 
 */



#include <zephyr/kernel.h>
#include <cstdlib>
#include <zephyr/logging/log.h>


LOG_MODULE_DECLARE(streamsched_apps);

#include "rtos_events.hpp"


#include "datatypes.hpp"

#include "appa_params.h"
#include "appb_params.h"
