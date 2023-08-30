/**
 * TODO: Add a description here
 * Author: Kasperi Kiviluoma
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <nrfx_rtc.h>

LOG_MODULE_REGISTER(nRF52DK, LOG_LEVEL_DBG);

const nrfx_rtc_t rtc = NRFX_RTC_INSTANCE(0);

int main(void) {
    nrfx_rtc_enable(&rtc);
    nrfx_rtc_tick_enable(&rtc, true);

    while (true) {
        LOG_DBG("%u", nrfx_rtc_counter_get(&rtc));
        k_msleep(1000);
    }
    return 0;
}