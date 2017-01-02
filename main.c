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

#include "gacal.h"
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>


/*
 * Output help/usage and exit the program
 */
static void usage(int exit_code) {
    puts("");
    puts("USAGE");
    puts("    acal [options] [ [day] [month] [year] ]");
    puts("");
    puts("DESCRIPTION");
    puts("    Another CALendar displays a basic calendar. If no arguments");
    puts("    are specified, the current month is displayed.");
    puts("");
    puts("OPTIONS");
    puts("    -s, --sunday  - use Sunday as the first day of the week");
    puts("    -m, --monday  - use Monday as the first day of the week");
    puts("    -h, --help    - display this help");
    puts("    -v, --version - output version information");
    puts("");
    puts("EXAMPLES");
    puts("    acal 12 2016");
    puts("    acal 20 07 1971");

    exit(exit_code);
}


/*
 * Ouput a version string and exit the program
 */
static void version(void) {
    printf("acal - Another CALendar %s (compiled %s)\n", VERSION, __DATE__);
    exit(0);
}


/*
 * Ouput an error message and exit the program with an error statuis code
 */
static void error(char *message) {
    puts(message);
    exit(-1);
}


/*
 * Parse the user given command line to set options
 */
static int get_command_line_options(
        int argc, char *argv[],
        struct Date *calendar,
        struct Date *highlight) {
    int optc = 0;

    static struct option const longopts[] = {
        {"sunday",   no_argument, NULL, 's'},
        {"monday",   no_argument, NULL, 'm'},
        {"help",     no_argument, NULL, 'h'},
        {"version",  no_argument, NULL, 'V'},
        {NULL, 0, NULL, 0}
    };

    while ((optc = getopt_long(argc, argv, "smhv", longopts, NULL)) != -1) {
        switch (optc) {
            case 's':
                set_weekday_start(SUNDAY);
                break;
            case 'm':
                set_weekday_start(MONDAY);
                break;
            case 'h':
                usage(0);
                break;
            case 'v':
                version();
                break;
            default:
                usage (-1);
        }
    }

    if (optind < argc) {
        if (argc-optind == 3) {
            highlight->day = atoi(argv[optind]);
            highlight->month = atoi(argv[optind+1]);
            highlight->year = atoi(argv[optind+2]);
                calendar->month = highlight->month;
            calendar->year = highlight->year;
        } else if (argc-optind == 2) {
                calendar->month = atoi(argv[optind]);
            calendar->year = atoi(argv[optind+1]);
        } else
            usage(-1);
    }

    if (calendar->month < 1 || calendar->month > 12)
        error("acal: illegal month value: use 1-12");

    if (calendar->year < 1 || calendar->year >= MAX_YEAR)
        error("acal: illegal year value: out of range");

    return optc;
}


int main(int argc, char *argv[]) {

    struct Date highlight_date = today();
    struct Date calendar_date = highlight_date;

    get_command_line_options(argc, argv,
        &calendar_date, &highlight_date);

    set_highlight_date(highlight_date);
    set_current_date(calendar_date);

    gtk_init(&argc, &argv);
    init_app();
    set_calendar();
    gtk_main();
    return 0;
}

