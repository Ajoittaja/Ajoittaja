/**
 * @file main.c
 * @brief Ajoittaja main
 * @author Kasperi Kiviluoma
 * @license MIT
 */

#include "schedules.h"
#include "system-time.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define THREAD_STACKSIZE 1024
#define THREAD_PRIORITY  7

// TODO: Allow to change log level
LOG_MODULE_REGISTER(Ajoittaja, LOG_LEVEL_DBG);

/**
 * TODO:
 * Remove, used for testing
 * The now variable is global; if possible, encapsulate it or limit its scope
 * The schedules array uses pointers to stack-allocated arrays; this is fine if
 * the arrays are never modified or freed, but consider using static or const
 * for safety Consider adding error handling or validation for schedule data Add
 * function documentation for schedule_handler (not shown here) If possible,
 * avoid magic numbers (like 3600, 7200, 10) by defining them as named constants
 */
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
K_THREAD_DEFINE(schedule_handler_id, THREAD_STACKSIZE, schedule_handler, NULL,
                NULL, NULL, THREAD_PRIORITY, 0, 0);