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
static void usage(int exit_code)
{
    puts("\nUSAGE");
    puts("    agcal [options] [ [day] [month] [year] ]\n");
    puts("DESCRIPTION");
    puts("    Another (g)CALendar displays a gui calendar. If no arguments");
    puts("    are specified, the current month is shown. Keys:");
    puts("        h|Left    - decrease month");
    puts("        l|Right   - increase month");
    puts("        k|Up      - increase year");
    puts("        j|Down    - decrease year\n");
    puts("        g|Home    - return to current date\n");
    puts("        q|Esc     - exit the calendar\n");
    puts("OPTIONS");
    puts("    -x, --xoffset  - move the window position +/- horizontally");
    puts("    -s, --yoffset  - move the window position +/- vertically");
    puts("    -m, --monday   - use Monday as the first day of the week");
    puts("    -h, --help     - display this help");
    puts("    -v, --version  - output version information\n");

    exit(exit_code);
}


/*
 * Ouput a version string and exit the program
 */
static void version(void)
{
    printf("gacal - Another (g)CALendar %s (compiled %s)\n", VERSION, __DATE__);
    exit(0);
}


/*
 * Ouput an error message and exit the program with an error statuis code
 */
static void error(char *message)
{
    puts(message);
    exit(-1);
}


/*
 * Parse the user given command line to set options
 */
static int get_command_line_options(
        int argc, char *argv[], Options *options)
{
    int optc = 0;

    static struct option const longopts[] = {
        {"xoffset",            1, NULL, 'x'},
        {"yoffset",            1, NULL, 'y'},
        {"sunday",   no_argument, NULL, 's'},
        {"monday",   no_argument, NULL, 'm'},
        {"help",     no_argument, NULL, 'h'},
        {"version",  no_argument, NULL, 'V'},
        {NULL, 0, NULL, 0}
    };

    while ((optc = getopt_long(argc, argv, "x:y:smhv", longopts, NULL)) != -1) {
        switch (optc) {
            case 'x':
               options->x_offset = atoi(optarg);
                break;
            case 'y':
                options->y_offset = atoi(optarg);
                break;
            case 's':
                options->week_start = SUNDAY;
                break;
            case 'm':
                options->week_start = MONDAY;;
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
            options->highlight.day = atoi(argv[optind]);
            options->highlight.month = atoi(argv[optind+1]);
            options->highlight.year = atoi(argv[optind+2]);
                options->month = options->highlight.month;
            options->year = options->highlight.year;
        } else if (argc-optind == 2) {
                options->month = atoi(argv[optind]);
            options->year = atoi(argv[optind+1]);
        } else
            usage(-1);
    }

    if (options->month < 1 || options->month > 12)
        error("gacal: illegal month value: use 1-12");

    if (options->year < 1 || options->year >= MAX_YEAR)
        error("gacal: illegal year value: out of range");

    return optc;
}


int main(int argc, char *argv[])
{
    Options options;

    options.highlight = today();
    options.month = options.highlight.month;
    options.year = options.highlight.year;
    options.week_start = MONDAY;
    options.x_offset = 0;
    options.y_offset = 0;

    get_command_line_options(argc, argv, &options);

    gtk_init(&argc, &argv);
    CalendarPtr calendar = create_calendar(options);
    gtk_main();
    destroy_calendar(calendar);
    return 0;
}

