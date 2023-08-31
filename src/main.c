/**
 * Ajoittaja main loop
 *
 * Author: Kasperi Kiviluoma
 * License: MIT
 */

#include <nrfx_rtc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// Log
LOG_MODULE_REGISTER(nRF52DK, LOG_LEVEL_DBG);   // TODO: Change name

// RTC
const nrfx_rtc_t rtc0 = NRFX_RTC_INSTANCE(0);
uint32_t hours = 21;
uint32_t minutes = 07;
uint32_t seconds = 0;

// TODO: Why static?
// TODO: Improve
static void rtc0_handler(nrfx_rtc_int_type_t int_type) {
    if (nrfx_rtc_counter_get(&rtc0) >= 8) {
        seconds++;
        if (seconds >= 60) {
            seconds = 0;
            minutes++;
            if (minutes >= 60) {
                minutes = 0;
                hours++;
                if (hours >= 24) {
                    hours = 0;
                }
            }
        }
        nrfx_rtc_counter_clear(&rtc0);
        LOG_INF("Time: %02d:%02d:%02d", hours, minutes, seconds);
    }
}

// Initialize RTC0
// TODO: Move to separate file
nrfx_err_t rtc0_init(void) {
    nrfx_err_t status;

    // Conf
    nrfx_rtc_config_t conf = NRFX_RTC_DEFAULT_CONFIG;
    // TODO: Improve?
    conf.prescaler = 4095;   // 32768 / 4096 = 8 Hz

    // Init
    status = nrfx_rtc_init(&rtc0, &conf, rtc0_handler);
    if (status == NRFX_SUCCESS) {
        nrfx_rtc_tick_enable(&rtc0, true);
        nrfx_rtc_enable(&rtc0);
        LOG_INF("rtc0_init success");
    }

    return status;
}

// TODO: Why?
// TODO: Move
static void manual_isr_setup() {
    IRQ_DIRECT_CONNECT(RTC0_IRQn, 0, nrfx_rtc_0_irq_handler, 0);
    irq_enable(RTC0_IRQn);
}

int main(void) {
    // Init RTC0
    if (rtc0_init() != NRFX_SUCCESS) {
        LOG_ERR("RTC0 init failed");
        return -1;
    }

    // Init IRQ
    manual_isr_setup();

    return 0;
}