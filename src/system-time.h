/**
 * @file system-time.h
 * @brief Time definitions
 * @author Kasperi Kiviluoma
 * @license MIT
 */

#pragma once

/**
 * TODO:
 * Consider using enum Weekday as the type for Time.day instead of uint8_t for
 * stronger type safety. Add documentation for the valid range of each field in
 * Time. If now should not be modified outside a specific module, consider
 * encapsulating it or providing getter/setter functions.
 */

/**
 * @brief The weekday enum
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
 * @brief The time struct
 *
 * Used to set the time of the schedule more easily.
 */
typedef struct {
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} Time;

/// The current time
extern Time system_time;