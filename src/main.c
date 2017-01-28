/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#include <gtk/gtk.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gui.h"
#include "common.h"
#include "instance.h"


/*
 * Output help/usage and exit the program
 */
static void usage(int exit_code)
{
    puts("\nUSAGE");
    puts("    galendae [OPTION ...] \n");
    puts("DESCRIPTION");
    puts("    Galandae displays a gui calendar. Keys:");
    puts("        h|Left    - decrease month");
    puts("        l|Right   - increase month");
    puts("        k|Up      - increase year");
    puts("        j|Down    - decrease year\n");
    puts("        g|Home    - return to current date\n");
    puts("        q|Esc     - exit the calendar\n");
    puts("OPTIONS");
    puts("    -c, --config FILE   - config file to load");
    puts("    -h, --help          - display this help and exit");
    puts("    -v, --version       - output version information\n");

    exit(exit_code);
}


/*
 * Output version information to the console and exit.
 */
static void version(void)
{
    printf(APP_NAME " version %s (compiled %s)\n", VERSION, __DATE__);
    exit(0);
}


/*
 * Process the commandline arguments passed to our app. This basic app
 * runs a GUI and only expects very 'basic' arguments, help, version and config.
 * more detailed options are provided by a config file.
 */
static char *process_arguments(int argc, char *argv[])
{
    int optc = 0;
    char *config_filename = NULL;
    static struct option const longopts[] = {
        {"config",   no_argument, NULL, 'c'},
        {"help",     no_argument, NULL, 'h'},
        {"version",  no_argument, NULL, 'V'},
        {NULL, 0, NULL, 0}
    };

    while ((optc = getopt_long(argc, argv, "c:hv", longopts, NULL)) != -1) {
        switch (optc) {
            case 'c':
                config_filename = strdup(optarg);
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

    return config_filename;
}


/*
 * Signal handler to end the existing instance cleanly
 */
static void signal_handler(int signal_id)
{
    if (signal_id == SIGTERM)
        gtk_main_quit();
}


/*
 * Create a main window and start the gtk event loop.
 */
int main(int argc, char *argv[])
{
    InstancePtr instance = instance_create();

    if (instance_is_unique(instance) != TRUE) {
        // Another instance is running
        instance_kill(instance);
    } else {
        // Continue with a unique instance of the program
        signal(SIGTERM, &signal_handler);
        gtk_init(&argc, &argv);
        char *config_filename = process_arguments(argc, argv);
        CalendarPtr window = create_calendar(config_filename);
        gtk_main();

        destroy_calendar(window);
        if (config_filename)
            free(config_filename);
    }

    instance_free(instance);
    return 0;
}
