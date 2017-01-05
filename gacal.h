/******************************************************************************
 *                                                                            *
 *          Another (g)CALendar Copyright (C) 2016 Chris Marsh                *
 *               <https://github.com/chris-marsh/gcalendar                    *
 *                                                                            *
 * This program is free software: you can redistribute it and/or modify it    *
 * under the terms of the GNU General Public License as published by the      *
 * Free Software Foundation, either version 3 of the License, or any later    *
 * version.                                                                   *
 *                                                                            *
 * This program is distributed in the hope that it will be useful, but        *
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY *
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   *
 * at <http://www.gnu.org/licenses/> for more details.                        *
 *                                                                            *
 ******************************************************************************/

#ifndef HEADERFILE_GACAL
#define HEADERFILE_GACAL

#include <limits.h>
#include <gtk/gtk.h>

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

#define VERSION "0.1"
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

typedef struct Calendar* CalendarPtr;

CalendarPtr create_calendar(Options options);
void destroy_calendar(CalendarPtr calendar);
Date today(void);

#endif
