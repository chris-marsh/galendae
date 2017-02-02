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

typedef struct Calendar *CalendarPtr;

CalendarPtr create_calendar(char *config_filename);
void destroy_calendar(CalendarPtr calendar);

#endif
