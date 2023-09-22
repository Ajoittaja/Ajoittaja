/**
 * @file main.c
 * @brief Ajoittaja main
 * @author Kasperi Kiviluoma
 * @license MIT
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "timer.h"

LOG_MODULE_REGISTER(Ajoittaja, LOG_LEVEL_DBG);

Time now = {thursday, 21, 59, 50};

Schedule mon[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule tue[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule wed[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule thu[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule fri[] = {{7, 0, 0, 3600}, {20, 0, 0, 7200}};
Schedule sat[] = {{20, 0, 0, 7200}};
Schedule sun[] = {{20, 0, 0, 7200}};
Schedule *schedules[] = {mon, tue, wed, thu, fri, sat, sun};

int main(void) {
    // Init GPIO
    if (gpio_init() != 0) {
        LOG_ERR("GPIO init failed");
        return -1;
    }

    // Init RTC0, 125ms counter period
    if (rtc_init(4095, time_rtc, time_handler, true) != NRFX_SUCCESS) {
        LOG_ERR("RTC0 init failed");
        return -1;
    }

    // Init RTC2
    if (rtc_init(4095, schedule_rtc, schedule_handler, false) != NRFX_SUCCESS) {
        LOG_ERR("RTC2 init failed");
        return -1;
    }

    // Init IRQs
    manual_isr_setup();

    // TODO: Low power sleep
    while (true) {
        k_sleep(K_MSEC(10));
    }

    return 0;   // Should never reach this
}