/**
 * @file timer.c
 * @brief Implementation of the Ajoittaja schedule handlers.
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Contains the implementation for handling the GPIO pin state based on the set
 * schedules. Keeps track of the current time.
 */

#include "timer.h"
#include <zephyr/logging/log.h>

/////////////
// Set up //
///////////

LOG_MODULE_DECLARE(Ajoittaja, LOG_LEVEL_DBG);

// Connect GPIO driver to this pin
struct gpio_dt_spec gpio_pin = GPIO_DT_SPEC_GET(DT_NODELABEL(led0), gpios);

// Turn GPIO state HIGH for the duration of the schedule
void gpio_high(uint32_t duration);

#define GPIO_LOW  1
#define GPIO_HIGH 0
bool gpio_state = GPIO_LOW;

const nrfx_rtc_t time_rtc = NRFX_RTC_INSTANCE(TIME_RTC_INSTANCE);
const nrfx_rtc_t schedule_rtc = NRFX_RTC_INSTANCE(SCHEDULE_RTC_INSTANCE);

///////////
// GPIO //
/////////

int gpio_init(void) {
    // Check if the device is ready
    if (!device_is_ready(gpio_pin.port)) {
        return -1;
    }

    // BUG: GPIO is HIGH if pin is set to logical 0
    gpio_pin_configure_dt(&gpio_pin, GPIO_OUTPUT_ACTIVE);

    return 0;
}

///////////////////////
// Real Time Clocks //
/////////////////////

nrfx_err_t rtc_init(int prescaler_val, const nrfx_rtc_t rtc,
                    nrfx_rtc_handler_t handler, bool tick_irq) {
    nrfx_err_t retval;

    // Set default conf, change prescaler and init
    nrfx_rtc_config_t conf = NRFX_RTC_DEFAULT_CONFIG;
    conf.prescaler = prescaler_val;
    retval = nrfx_rtc_init(&rtc, &conf, handler);

    // Enable tick event and start RTC
    if (retval == NRFX_SUCCESS) {
        nrfx_rtc_tick_enable(&rtc, tick_irq);
        nrfx_rtc_enable(&rtc);
    }

    return retval;
}

void manual_isr_setup() {
    IRQ_DIRECT_CONNECT(RTC0_IRQn, 0, nrfx_rtc_0_irq_handler, 0);
    irq_enable(RTC0_IRQn);

    IRQ_DIRECT_CONNECT(RTC2_IRQn, 0, nrfx_rtc_2_irq_handler, 0);
    irq_enable(RTC2_IRQn);
}

void time_handler(nrfx_rtc_int_type_t int_type) {
    // Keep track of the current time
    if (int_type == NRFX_RTC_INT_TICK) {
        // Counter is incremented every 125 ms
        if (nrfx_rtc_counter_get(&time_rtc) >= 8) {
            now.second++;
            if (now.second >= 60) {
                now.second = 0;
                now.minute++;
                if (now.minute >= 60) {
                    now.minute = 0;
                    now.hour++;
                    if (now.hour >= 24) {
                        now.hour = 0;
                        now.day++;
                        if (now.day >= sunday) {
                            now.day = monday;
                        }
                    }
                }
            }
            // Needs to be cleared to keep track of the seconds passed
            nrfx_rtc_counter_clear(&time_rtc);

            LOG_INF("Time: %02d:%02d:%02d", now.hour, now.minute, now.second);

            // Check if any of the schedules should be triggered
            // schedules is organized so that schedules[0] is monday, etc..
            if (gpio_state == GPIO_LOW) {
                int i = now.day;
                for (int j = 0; j < SCHEDULE_SIZE; j++) {
                    // Calculate the total seconds for the start and current
                    // time Without this the GPIO driver is not turned HIGH if
                    // Ajoittaja was turned HIGH after the start time
                    int start_seconds = schedules[i][j].start_hour * 3600 +
                                        schedules[i][j].start_minute * 60 +
                                        schedules[i][j].start_second;
                    int now_seconds =
                        now.hour * 3600 + now.minute * 60 + now.second;

                    // Check if the current time matches or is within the
                    // scheduled duration
                    if (now_seconds >= start_seconds &&
                        now_seconds <
                            start_seconds + schedules[i][j].duration) {

                        // Calculate the remaining duration the GPIO driver
                        // should be in HIGH state
                        int remaining_duration = start_seconds +
                                                 schedules[i][j].duration -
                                                 now_seconds;

                        gpio_high(remaining_duration);
                        break;
                    }
                }
            }
        }
    }
}

void schedule_handler(nrfx_rtc_int_type_t int_type) {
    LOG_INF("Schedule handler");
    if (int_type == NRFX_RTC_INT_COMPARE0) {
        LOG_INF("Turn GPIO to LOW state");
        gpio_state = GPIO_LOW;
        gpio_pin_set_dt(&gpio_pin, GPIO_LOW);
    }
}

void gpio_high(uint32_t duration) {
    // Clear counter to start counting from 0
    nrfx_rtc_counter_clear(&schedule_rtc);

    // Set compare value to trigger interrupt when it is time to turn GPIO state
    // LOW Convert from seconds to ticks
    if (nrfx_rtc_cc_set(&schedule_rtc, 0, duration * 8, true) != NRFX_SUCCESS) {
        LOG_ERR("Failed to set compare value for the schedule RTC");
    } else {
        LOG_INF("Turn GPIO to HIGH state");
        gpio_state = GPIO_HIGH;
        gpio_pin_set_dt(&gpio_pin, GPIO_HIGH);
    }
}