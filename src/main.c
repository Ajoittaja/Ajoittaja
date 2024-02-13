/**
 * @file main.c
 * @brief Ajoittaja main
 * @author Kasperi Kiviluoma
 * @license MIT
 */

#include "bluetooth.h"
#include "schedules.h"
#include "time-now.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define STACKSIZE       1024
#define BLE_PRIORITY    6
#define LOWEST_PRIORITY 7

LOG_MODULE_REGISTER(Ajoittaja, LOG_LEVEL_DBG);

// TODO: Remove, used for testing
Time now = {friday, 21, 59, 55};
Schedule mon[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule tue[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule wed[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule thu[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule fri[] = {{7, 0, 0, 3600}, {22, 0, 0, 10}};
Schedule sat[] = {{20, 0, 0, 7200}};
Schedule sun[] = {{20, 0, 0, 7200}};
Schedule *schedules[] = {mon, tue, wed, thu, fri, sat, sun};

// Schedules and current time handling
K_THREAD_DEFINE(schedule_handler_id, STACKSIZE, schedule_handler, NULL, NULL,
                NULL, LOWEST_PRIORITY, 0, 0);

// Bluetooth handling
K_THREAD_DEFINE(ble_handler_id, STACKSIZE, ble_handler, NULL, NULL, NULL,
                BLE_PRIORITY, 0, 0);