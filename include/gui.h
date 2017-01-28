/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#ifndef GUI_H
#define GUI_H

#include <limits.h>

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#define MAX_YEAR (INT_MAX / 12 - 1)  /* Limited by the inc_month function */

typedef enum{
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY
} Days_of_Week;

typedef struct {
    unsigned int day;
    unsigned int month;
    unsigned int year;
} Date;

typedef struct {
    int x_offset;
    int y_offset;
    int month;
    int year;
    Date highlight;
    Days_of_Week week_start;
} Options;

typedef struct Calendar *CalendarPtr;

CalendarPtr create_calendar(char *config_filename);
void destroy_calendar(CalendarPtr calendar);

#endif
