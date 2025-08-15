/**
 * @file main.c
 * @brief Ajoittaja main
 * @author Kasperi Kiviluoma
 * @license MIT
 */

#include "bluetooth.h"
#include "my_lbs.h"
#include "schedules.h"
#include "time-now.h"
#include <dk_buttons_and_leds.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define STACKSIZE       1024
#define BLE_PRIORITY    6
#define LOWEST_PRIORITY 7

#define NOTIFY_INTERVAL 500
#define PRIORITY        7
static uint32_t app_sensor_value = 100;

static void simulate_data(void) {
    app_sensor_value++;
    if (app_sensor_value == 200) {
        app_sensor_value = 100;
    }
}

void send_data_thread(void) {
    while (1) {
        /* Simulate data */
        simulate_data();
        /* Send notification, the function sends notifications only if a client
         * is subscribed */
        my_lbs_send_sensor_notify(app_sensor_value);

        k_sleep(K_MSEC(NOTIFY_INTERVAL));
    }
}

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

K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL,
                NULL, PRIORITY, 0, 0);