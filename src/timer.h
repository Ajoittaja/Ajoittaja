/**
 * @file timer.h
 * @brief Handles Ajoittaja schedules.
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Turn the GPIO pin high/low based on the set schedules.
 */

#pragma once

#include <nrfx_rtc.h>
#include <zephyr/drivers/gpio.h>

///////////
// GPIO //
/////////

/**
 * Initialize the GPIO pin.
 *
 * @retval 0  Initialization successful.
 * @retval -1 Initialization failed.
 */
int gpio_init(void);

///////////
// Time //
/////////

/**
 * @brief The weekday enum.
 *
 * Used to set day of the schedule more easily.
 */
enum Weekday {
    monday,
    tuesday,
    wednesday,
    thursday,
    friday,
    saturday,
    sunday,
};

/**
 * @brief The time struct.
 *
 * Used to set the time of the schedule more easily.
 */
typedef struct {
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Time;

/// The current time.
extern Time now;

////////////////
// Schedules //
//////////////

#define SCHEDULE_SIZE 3

/**
 * @brief Contains the start time and duration.
 *
 * One schedule array contains all the start times for the day.
 */
typedef struct {
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t start_second;
    uint32_t duration;
} Schedule;

/// Monday schedule.
extern Schedule mon[];
/// Tuesday schedule.
extern Schedule tue[];
/// Wednesday schedule.
extern Schedule wed[];
/// Thursday schedule.
extern Schedule thu[];
/// Friday schedule.
extern Schedule fri[];
/// Saturday schedule.
extern Schedule sat[];
/// Sunday schedule.
extern Schedule sun[];

/// Array of pointers to the daily schedules.
extern Schedule *schedules[];

///////////////////////
// Real Time Clocks //
/////////////////////

#define TIME_RTC_INSTANCE     0
#define SCHEDULE_RTC_INSTANCE 2
extern const nrfx_rtc_t time_rtc;
extern const nrfx_rtc_t schedule_rtc;

/**
 * @brief Initializes the RTC module.
 *
 * Uses the given prescaler value to configure the RTC's tick rate.
 *
 * @param prescaler_val Prescaler value to configure the RTC's TICK event rate.
 * @param rtc           A pointer to an instance of the RTC to be initialized.
 * @param handler       Event handler callback function for RTC-related events.
 * @param tick_irq      Enable/disable tick event interrupt.
 *
 * @retval NRFX_SUCCESS             Successfully initialized.
 * @retval NRFX_ERROR_INVALID_STATE The instance is already initialized.
 */
nrfx_err_t rtc_init(int prescaler_val, const nrfx_rtc_t rtc,
                    nrfx_rtc_handler_t handler, bool tick_irq);

/// Setup RTC interrupt handlers
void manual_isr_setup();

/**
 * @brief Handler for the time RTC interrupts.
 *
 * Keeps track of the current time. If the current time matches to the start
 * time of a schedule turn the GPIO state HIGH and set the compare value to
 * trigger an interrupt when it is time to turn LOW.
 *
 * @param irq_type Type of the interrupt.
 */
void time_handler(nrfx_rtc_int_type_t irq_type);

/**
 * If triggered by a compare event, turns the GPIO state LOW.
 *
 * @param irq_type Type of the interrupt.
 */
void schedule_handler(nrfx_rtc_int_type_t int_type);