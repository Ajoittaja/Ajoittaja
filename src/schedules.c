/**
 * @file schedules.c
 * @brief Implementation of the Ajoittaja schedules handler
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Contains the implementation for handling the GPIO pin state based on the set
 * schedules. Keeps track of the current time.
 */

#include "schedules.h"

#include "system-time.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define GPIO_LOW  0
#define GPIO_HIGH 1

/**
 * TODO:
 * The schedules variable is used as an external array; ensure it is defined and
 * initialized in one source file (likely main.c). Consider using const for
 * schedule arrays if they are not modified at runtime. Magic numbers (like
 * 3600, 60, etc.) could be replaced with named constants for clarity. Add error
 * handling for timer initialization and GPIO operations. Consider documenting
 * the expected range and meaning of now.day (e.g., enum values for weekdays).
 * If possible, move GPIO-specific code to a separate module for better
 * separation of concerns.
 */

// TODO: Allow to change log level
LOG_MODULE_DECLARE(Ajoittaja, LOG_LEVEL_DBG);

struct gpio_dt_spec gpio_pin =
    GPIO_DT_SPEC_GET(DT_NODELABEL(led2), gpios);   // TODO: Make configurable
struct k_timer gpio_low_timer;

void schedule_handler() {
    bool stop = false;
    k_timer_init(&gpio_low_timer, gpio_set_low, NULL);

    if (gpio_init() != 0) {
        LOG_ERR("GPIO init failed");
        stop = true;
    }

    while (!stop) {
        k_sleep(K_SECONDS(1));
        system_time.second++;
        update_system_time();
        check_schedules();
    }
}

void update_system_time() {
    if (system_time.second >= 60) {
        system_time.second = 0;
        system_time.minute++;
        if (system_time.minute >= 60) {
            system_time.minute = 0;
            system_time.hour++;
            if (system_time.hour >= 24) {
                system_time.hour = 0;
                system_time.day++;
                if (system_time.day >= sunday) {
                    system_time.day = monday;
                }
            }
        }
    }
    LOG_INF("Time: %02d:%02d:%02d", system_time.hour, system_time.minute,
            system_time.second);
}

void check_schedules() {
    // Check if any of the schedules should be triggered
    // schedules is organized so that schedules[0] is monday, etc..
    if (k_timer_remaining_get(&gpio_low_timer) == 0) {
        int i = system_time.day;
        for (int j = 0; j < SCHEDULE_SIZE; j++) {
            // Convert schedule start time and current time to seconds
            int start_seconds = schedules[i][j].start_hour * 3600 +
                                schedules[i][j].start_minute * 60 +
                                schedules[i][j].start_second;
            int now_seconds = system_time.hour * 3600 +
                              system_time.minute * 60 + system_time.second;

            // Check if the current time matches or is within the
            // scheduled duration
            // This ensures that even if the device is started in the middle of
            // the schedule, schedule is still activated
            if (now_seconds >= start_seconds &&
                now_seconds < start_seconds + schedules[i][j].duration) {

                LOG_INF("Turn GPIO to HIGH state");
                gpio_pin_set_dt(&gpio_pin, GPIO_HIGH);

                // Calculate the remaining duration the GPIO driver
                // should be in HIGH state
                int remaining_duration =
                    start_seconds + schedules[i][j].duration - now_seconds;

                if (remaining_duration > 0) {
                    k_timer_start(&gpio_low_timer,
                                  K_SECONDS(remaining_duration), K_NO_WAIT);
                }
                break;
            }
        }
    }
}

int gpio_init(void) {
    // Check if the device is ready
    if (!device_is_ready(gpio_pin.port)) {
        return -1;
    }
    gpio_pin_configure_dt(&gpio_pin, GPIO_ACTIVE_HIGH | GPIO_OUTPUT_INACTIVE);
    return 0;
}

void gpio_set_low() {
    LOG_INF("Turn GPIO to LOW state");
    gpio_pin_set_dt(&gpio_pin, GPIO_LOW);
}