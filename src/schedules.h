/**
 * @file schedules.h
 * @brief Handles Ajoittaja schedules
 * @author Kasperi Kiviluoma
 * @license MIT
 *
 * Turn the GPIO pin high/low based on the set schedules.
 */

#pragma once

#include <zephyr/drivers/gpio.h>

#define SCHEDULE_SIZE 3   // Max schedules in a day

/**
 * @brief Contains the start time and duration
 *
 * One schedule array contains all the start times for the day.
 */
typedef struct {
    uint8_t start_hour;
    uint8_t start_minute;
    uint8_t start_second;
    uint32_t duration;
} Schedule;

/// Monday schedule
extern Schedule mon[];
/// Tuesday schedule
extern Schedule tue[];
/// Wednesday schedule
extern Schedule wed[];
/// Thursday schedule
extern Schedule thu[];
/// Friday schedule
extern Schedule fri[];
/// Saturday schedule
extern Schedule sat[];
/// Sunday schedule
extern Schedule sun[];

/// Array of pointers to the daily schedules
extern Schedule *schedules[];

/// Keep track of the time and turn gpio on/off based on the schedules
void schedule_handler();

/// Update the current time
void update_now();

/**
 * @brief Toggle gpio based on the schedules
 *
 * If current time is within the schedule turn gpio HIGH and set timer to turn
 * it LOW.
 */
void check_schedules();

/**
 * @brief Initialize the GPIO pin
 *
 * @retval 0  Initialization successful
 * @retval -1 Initialization failed
 */
int gpio_init(void);

/// Timer callback to set gpio LOW
void gpio_set_low();