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

// Max schedules in a day
#define SCHEDULE_SIZE 3

/**
 * TODO:
 * Consider using const for schedule arrays if they are not modified at runtime
 * SCHEDULE_SIZE is defined but not used in this header; ensure it is used
 * consistently in implementation If possible, group related functions (e.g.,
 * GPIO functions) in a separate header for better modularity Add parameter
 * documentation for functions that take arguments Consider documenting expected
 * time units for duration (e.g., seconds)
 */

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

extern Schedule mon[];
extern Schedule tue[];
extern Schedule wed[];
extern Schedule thu[];
extern Schedule fri[];
extern Schedule sat[];
extern Schedule sun[];

/**
 * @brief Array of pointers to the daily schedules
 */
extern Schedule *schedules[];

/**
 * @brief Keep track of the time and turn gpio on/off based on the schedules
 */
void schedule_handler();

/**
 * @brief Update the current time
 */
void update_system_time();

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

/**
 * @brief Timer callback to set gpio LOW
 */
void gpio_set_low();