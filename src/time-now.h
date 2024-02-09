/**
 * @file time-now.h
 * @brief Time definitions
 * @author Kasperi Kiviluoma
 * @license MIT
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
extern Time now;