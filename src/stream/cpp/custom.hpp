/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream Library
 * Title:        custom.h
 * Description:  Example configuration for CMSIS-Stream with event handling in bare metal
 *               and multi-threaded environments.
 *
 *
 * Target Processor: Cortex-M and Cortex-A cores
 * --------------------------------------------------------------------
 *
 * Copyright (C) 2021-2025 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CUSTOM_H_
#define CUSTOM_H_

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stream_scheduler_module);

#include "stream_types.hpp"


#endif