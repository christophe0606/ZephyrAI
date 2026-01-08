/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

#include "stream.h"

int main(void)
{
    LOG_INF("Starting main\n");

    int err = init_stream();
    if (err != 0) {
        LOG_INF("Error initializing stream\n");
        return -1;
    }

    return 0;
}
