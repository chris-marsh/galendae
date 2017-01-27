/******************************************************************************
 *                                                                            *
 *                galandae copyright (c) 2016 Chris Marsh                     *
 *               <https://github.com/chris-marsh/gcalendar                    *
 *                                                                            *
 * this program is free software: you can redistribute it and/or modify it    *
 * under the terms of the gnu general public license as published by the      *
 * free software foundation, either version 3 of the license, or any later    *
 * version.                                                                   *
 *                                                                            *
 * this program is distributed in the hope that it will be useful, but        *
 * without any warranty; without even the implied warranty of merchantability *
 * or fitness for a particular purpose.  see the gnu general public license   *
 * at <http://www.gnu.org/licenses/> for more details.                        *
 *                                                                            *
 ******************************************************************************/


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
Date today(void);

#endif
