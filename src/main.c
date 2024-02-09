/**
 * @file main.c
 * @brief Ajoittaja main
 * @author Kasperi Kiviluoma
 * @license MIT
 */

#include "schedules.h"
#include "time-now.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// Size of stack area used by each thread
#define STACKSIZE 1024
// Scheduling priority used by each thread
#define PRIORITY 7

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
                NULL, PRIORITY, 0, 0);