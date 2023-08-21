/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0a
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(Less4_Exer2, LOG_LEVEL_DBG);

int main(void) {
    printk("Hello World! %s\n", CONFIG_BOARD);
    LOG_DBG("nRF Connect SDK Fundamentals");
    return 0;
}
