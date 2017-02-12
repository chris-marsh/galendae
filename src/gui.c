/*
 * Project: Galendae Popup Calendar
 * URL:     https://github.com/chris-marsh/galendae
 * License: The MIT License
 */


#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "gui.h"
#include "config.h"
#include "common.h"


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


struct Weekdays {
    char *longname;
    char *shortname;
} weekdays[] = {
    {"Sunday", "Su"},
    {"Monday", "Mo"},
    {"Tuesday", "Tu"},
    {"Wednesday", "We"},
    {"Thursday", "Th"},
    {"Friday", "Fr"},
    {"Saturday", "Sa"}
};


struct Month {
    char *longName;
    unsigned int num_days;
} months[] = {
    {"January", 31},
    {"February", 28},
    {"March", 31},
    {"April", 30},
    {"May", 31},
    {"June", 30},
    {"July", 31},
    {"August", 31},
    {"September", 30},
    {"October", 31},
    {"November", 30},
    {"December", 31}
};


typedef struct {
    unsigned int row;
    unsigned int column;
} Point;


struct Calendar {
    GtkWidget *window;
    GtkWidget *drawing_area;
    GtkWindowPosition position;
    unsigned int month;
    unsigned int year;
    Days_of_Week week_start;
    Date highlight_date;
    Point highlight_cell;
    int x_offset;
    int y_offset;
    char *background_color;
    char *foreground_color;
    char *fringe_date_color;
    char *highlight_color;
    char *month_font_size;
    char *month_font_weight;
    char *day_font_size;
    char *day_font_weight;
    char *date_font_size;
    char *date_font_weight;
    char *arrow_font_size;
    char *arrow_font_weight;
    int close_on_unfocus;   /* 0=No , 1=Yes */
    int center;             /* 0=No , 1=Yes */
    int undecorated;        /* 0=No , 1=Yes */
    int stick;              /* 0=No , 1=Yes */
};


/*
 * Return struct with current system date
 */
Date today(void)
{
    time_t t = time(0);
    struct tm *now = localtime(&t);
    Date date = {
        now->tm_mday,
        now->tm_mon+1,
        now->tm_year+1900
    };
    return date;
}


/*
 * Add -/+num to month and inc/dec year as necessary. To simplify the
 * calculation, months are converted to 0 base and years converted to months.
 * Returns 0 if succesful, 1 indicates a fail, eg dec date beyond year 1.
 */
static int inc_month(int num, unsigned int *month, unsigned int *year)
{
    int tempYear  = *year;
    int tempMonth = *month - 1;

    *year = (tempYear * 12 + tempMonth + num) / 12;
    *month = (tempYear * 12 + tempMonth + num) % 12 + 1;

    if (*year < 1 || *month < 1) {
        *year = tempYear;
        *month = tempMonth + 1;
        return 0;
    } else {
        return 1;
    }
}


/*
 * Given month and year, calculate the day of week of the first day.
 * Day is returned as an integer 0-6, Sun-Sat
 * Based on Zeller's congruence;
 * https://en.wikipedia.org/wiki/Zeller's_congruence
 */
static int first_day_of_month(int month, int year)
{
    if (month < 3) {
        month += 12;
        year--;
    }
    int century = year / 100;
    year = year % 100;
    return (((13 * (month + 1)) / 5) +
            (century / 4) + (5 * century) +
            year + (year / 4)) % 7;
}


/*
 * Given the int year, returns TRUE if a leap year
 */
static int is_leap_year(const int year)
{
    if ((!(year % 4) && (year % 100)) || !(year % 400))
        return TRUE;
    else
        return FALSE;
}

/*
 * Update the calendar display
 */
static void update_calendar(CalendarPtr this)
{
    GtkWidget *grid;
    GtkWidget *label;
    char temp[20];

    grid = gtk_bin_get_child(GTK_BIN(this->window));
    if (grid == NULL)
        return;

    label = gtk_grid_get_child_at(GTK_GRID(grid), 1,0);
    sprintf(temp, "%s %d", months[this->month-1].longName, this->year);
    gtk_label_set_text(GTK_LABEL(label), temp);

    if (this->month == 2) {
        if (is_leap_year(this->year))
            months[this->month-1].num_days = 29;
        else
            months[this->month-1].num_days = 28;
    }

    unsigned int prev_month = this->month;
    unsigned int prev_year = this->year;
    inc_month(-1, &prev_month, &prev_year);

    if ((prev_month == 2) && is_leap_year(prev_year))
        months[prev_month-1].num_days = 29;

    int day = 1 - (first_day_of_month(this->month, this->year) -
            this->week_start + 7) % 7;

    for (int line = 2; line <=7; line++) {
        for (int col = 0; col < 7; col++) {
            label = gtk_grid_get_child_at(GTK_GRID(grid), col, line);

            if (day > 0 && day <= (int)months[this->month-1].num_days) {
                gtk_widget_set_name(GTK_WIDGET(label), "date");
                sprintf(temp, "%d", day);
            } else {
                gtk_widget_set_name(GTK_WIDGET(label), "fringeDate");
                if (day < 1)
                    sprintf(temp, "%d", months[prev_month-1].num_days+day);
                else
                    sprintf(temp, "%d", day - months[this->month-1].num_days);
            }

            gtk_label_set_text(GTK_LABEL(label), temp);
            if (day == (int)this->highlight_date.day &&
                    this->month == this->highlight_date.month &&
                    this->year == this->highlight_date.year) {
                this->highlight_cell.column = col+1;
                this->highlight_cell.row = line-1;
            }
            day++;
        }
    }
    gtk_widget_queue_draw_area(this->drawing_area, 0, 0,
            gtk_widget_get_allocated_width(GTK_WIDGET(this->drawing_area)),
            gtk_widget_get_allocated_height(GTK_WIDGET(this->drawing_area)));
}

/*
 * Give visual feedback by changing the mouse pointer when over an arrow
 */
static void on_arrow_hover(GtkWidget *UNUSED(eventbox), GdkEvent *event, CalendarPtr this)
{
    GdkCursor *cursor;
    GdkWindow *window;

    if (event->type == GDK_ENTER_NOTIFY) {
        cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_HAND2);
    } else {
        cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_ARROW);
    }

    window = gtk_widget_get_window(GTK_WIDGET(this->window));
    gdk_window_set_cursor(window, cursor);
}


/*
 * Inc/Dec the month and update the display
 */
static void on_arrow_click(GtkWidget *eventbox, GdkEvent *UNUSED(event), CalendarPtr this)
{
    int success = 0;
    if (gtk_widget_get_name(GTK_WIDGET(eventbox))[0] == 'l') {
        success = inc_month(-1, &this->month, &this->year);
    } else {
        success = inc_month(1, &this->month, &this->year);
    }
    if (success) update_calendar(this);
}


/*
 * Handle keypresses
 */
static void on_key_press (GtkWidget *window, GdkEventKey *eventkey, CalendarPtr this)
{
    switch (eventkey->keyval) {
        case GDK_KEY_q:
        case GDK_KEY_Escape:
            gtk_widget_destroy(window);
            break;
        case GDK_KEY_h:
        case GDK_KEY_Left:
            if (inc_month(-1, &this->month, &this->year))
                update_calendar(this);
            break;
        case GDK_KEY_l:
        case GDK_KEY_Right:
            if (inc_month(1, &this->month, &this->year))
                update_calendar(this);
            break;
        case GDK_KEY_k:
        case GDK_KEY_Up:
            if (this->year < MAX_YEAR) {
                this->year++;
                update_calendar(this);
            }
            break;
        case GDK_KEY_j:
        case GDK_KEY_Down:
            if (this->year > 1) {
                this->year--;
                update_calendar(this);
            }
            break;
        case GDK_KEY_g:
        case GDK_KEY_Home:
            this->month = this->highlight_date.month;
            this->year = this->highlight_date.year;
            update_calendar(this);
    }
}


/*
 * Resize drawing area so it is a perfect multiple of columns and rows
 */
static void resize_drawing_area (GtkWidget *widget)
{
    gint widget_width = gtk_widget_get_allocated_width(GTK_WIDGET(widget));
    gint widget_height = gtk_widget_get_allocated_height(GTK_WIDGET(widget));
    while (widget_width % 14) widget_width += 1;
    while (widget_height % 12) widget_height += 1;
    gtk_widget_set_size_request(GTK_WIDGET(widget), widget_width, widget_height);
}


/*
 * Highlight a date by drawing a circle around it
 */
static gboolean draw_callback (GtkWidget *widget, cairo_t *cr, CalendarPtr this)
{
    GdkRGBA color;
    gdk_rgba_parse(&color, this->highlight_color);
    gdk_cairo_set_source_rgba(cr, &color);

    int cell_width = gtk_widget_get_allocated_width(GTK_WIDGET(widget)) / 7;
    int cell_height = gtk_widget_get_allocated_height(GTK_WIDGET(widget)) / 6;

    if (this->month == this->highlight_date.month &&
            this->year == this->highlight_date.year) {
        cairo_arc(cr,
                (this->highlight_cell.column*cell_width)-(cell_width/2),
                (this->highlight_cell.row*cell_height)-(cell_height/2),
                (cell_height/2) - 1,
                0, 2 * G_PI);
    }
    cairo_stroke(cr);

    return FALSE;
}


/*
 * Create the main window and add all needed widgets
 */
GtkWidget* init_widgets(CalendarPtr this)
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *eventbox;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calendar");
    if (this->undecorated == 1)
        gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    if (this->stick)
        gtk_window_stick(GTK_WINDOW(window));
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);

    gtk_widget_set_name(GTK_WIDGET(window), "mainWindow");
    g_signal_connect(GTK_WINDOW(window), "key_press_event", G_CALLBACK(on_key_press), this);
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    if (this->close_on_unfocus == 1)
        g_signal_connect(GTK_WINDOW(window),
                "focus_out_event", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_widget_set_name(GTK_WIDGET(grid), "grid");
    gtk_container_add(GTK_CONTAINER(window), grid);

    eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(eventbox), "left_eventbox");
    g_signal_connect(GTK_WIDGET(eventbox), "enter-notify-event", G_CALLBACK(on_arrow_hover), this);
    g_signal_connect(GTK_WIDGET(eventbox), "leave-notify-event", G_CALLBACK(on_arrow_hover), this);
    g_signal_connect(GTK_WIDGET(eventbox), "button-release-event", G_CALLBACK(on_arrow_click), this);
    gtk_grid_attach(GTK_GRID(grid), eventbox, 0,0,1,1);

    label = gtk_label_new("<");
    gtk_widget_set_name(GTK_WIDGET(label), "leftArrow");
    gtk_grid_attach(GTK_GRID(grid), label, 0,0,1,1);

    label = gtk_label_new("Month Year");
    gtk_widget_set_name(GTK_WIDGET(label), "monthLabel");
    gtk_grid_attach(GTK_GRID(grid), label, 1,0,5,1);

    eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(eventbox), "right_eventbox");
    g_signal_connect(GTK_WIDGET(eventbox), "enter-notify-event", G_CALLBACK(on_arrow_hover), this);
    g_signal_connect(GTK_WIDGET(eventbox), "leave-notify-event", G_CALLBACK(on_arrow_hover), this);
    g_signal_connect(GTK_WIDGET(eventbox), "button-release-event", G_CALLBACK(on_arrow_click), this);
    gtk_grid_attach(GTK_GRID(grid), eventbox, 6,0,1,1);

    label = gtk_label_new(">");
    gtk_widget_set_name(GTK_WIDGET(label), "rightArrow");
    gtk_grid_attach(GTK_GRID(grid), label, 6,0,1,1);

    for (int day =0; day<7; day++) {
        label = gtk_label_new(weekdays[(day+this->week_start) % 7].shortname);
        gtk_widget_set_name(GTK_WIDGET(label), "weekdayLabel");
        gtk_widget_set_margin_top(GTK_WIDGET(label), 15);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 15);
        gtk_grid_attach(GTK_GRID(grid), label, day,1,1,1);
    }

    this->drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_name(GTK_WIDGET(this->drawing_area), "drawArea");
    g_signal_connect (G_OBJECT (this->drawing_area), "draw", G_CALLBACK (draw_callback), this);
    g_signal_connect(GTK_WIDGET(this->drawing_area), "realize", G_CALLBACK(resize_drawing_area), NULL);
    gtk_grid_attach(GTK_GRID(grid), this->drawing_area, 0, 2, 7, 6);

    for (int week = 2; week<8; week++) {
        for (int day=0; day<7; day++) {
            label = gtk_label_new(NULL);
            gtk_widget_set_margin_bottom(GTK_WIDGET(label),5);
            gtk_widget_set_margin_top(GTK_WIDGET(label),5);
            gtk_widget_set_margin_start(GTK_WIDGET(label),11);
            gtk_widget_set_margin_end(GTK_WIDGET(label),11);
            gtk_grid_attach(GTK_GRID(grid), label, day, week, 1, 1);
        }
    }

    return window;
}


/*
 * Set the user-defineable styles
 */
void set_style(CalendarPtr this)
{
    char *format = 
            "#mainWindow {background-color:%s; color:%s; font-size:%s; font-weight:%s;}"
            "#monthLabel {font-size:%s; font-weight:%s;}"
            "#weekdayLabel {font-size:%s; font-weight:%s;}"
            "#leftArrow, #rightArrow {font-size:%s; font-weight:%s;}"
            "#fringeDate {color:%s;}";
    int size = strlen(format) - 22 +
        strlen(this->background_color) + strlen(this->foreground_color) +
        strlen(this->date_font_size) + strlen(this->date_font_weight) +
        strlen(this->month_font_size) + strlen(this->month_font_weight) +
        strlen(this->day_font_size) + strlen(this->day_font_weight) +
        strlen(this->arrow_font_size) + strlen(this->arrow_font_weight) +
        strlen(this->fringe_date_color);
    char *style_data = malloc(size + 1);
    if (style_data) {
        sprintf(style_data, format,
                this->background_color, this->foreground_color,
                this->date_font_size, this->date_font_weight,
                this->month_font_size, this->month_font_weight,
                this->day_font_size, this->day_font_weight,
                this->arrow_font_size, this->arrow_font_weight,
                this->fringe_date_color);
        GtkCssProvider *provider = gtk_css_provider_new();
        gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
                style_data, strlen(style_data), NULL);
        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
        g_object_unref(provider);
        free(style_data);
    }
}


/*
 * Set the window position with users offsets andshow
 */
static void show_calendar(CalendarPtr this)
{
    gint x_pos = 0, y_pos = 0;

    gtk_window_set_position(GTK_WINDOW(this->window), this->position);
    gtk_window_get_position(GTK_WINDOW(this->window), &x_pos, &y_pos);
    gtk_window_move(GTK_WINDOW(this->window), this->x_offset + x_pos,  this->y_offset + y_pos);

    gtk_widget_show_all(this->window);
}


/*
 * Provide default values if no config file is found
 */
void set_default_config(CalendarPtr this)
{
    this->highlight_date = today();
    this->month = this->highlight_date.month;
    this->year = this->highlight_date.year;
    this->week_start = MONDAY;

    this->undecorated = 0;
    this->stick=0;
    this->close_on_unfocus = 0;
    this->position = GTK_WIN_POS_NONE;
    this->x_offset = 0;
    this->y_offset = 0;

    this->background_color = strdup("#afafaf");
    this->foreground_color = strdup("#000000");
    this->highlight_color = strdup("#2f2f2f");
    this->fringe_date_color = strdup("#717171");

    this->month_font_size = strdup("1.0em");
    this->month_font_weight = strdup("normal");

    this->day_font_size = strdup("1.0em");
    this->day_font_weight = strdup("normal");

    this->date_font_size = strdup("1.0em");
    this->date_font_weight = strdup("normal");

    this->arrow_font_size = strdup("1.0em");
    this->arrow_font_weight = strdup("normal");
}


/*
 * Create an instance of the calendar window and load/set config options 
 */
CalendarPtr create_calendar(char *config_filename)
{
    CalendarPtr this = malloc(sizeof *this);

    if (this) {
        set_default_config(this);

        Config *config = read_config_file(config_filename);
        if (config) {
            Option option;
            while (config != NULL) {
                option = pop_option(&config);

                if (strcmp(option.key, "undecorated") == 0)
                    this->undecorated = atoi(option.value);
                else if (strcmp(option.key, "stick") == 0)
                    this->stick = atoi(option.value);
                else if (strcmp(option.key, "close_on_unfocus") == 0)
                    this->close_on_unfocus = atoi(option.value);

                else if (strcmp(option.key, "x_offset") == 0)
                    this->x_offset = atoi(option.value);
                else if (strcmp(option.key, "y_offset") == 0)
                    this->y_offset = atoi(option.value);

                else if (strcmp(option.key, "background_color") == 0)
                    strfcpy(option.value, &this->background_color);
                else if (strcmp(option.key, "foreground_color") == 0)
                    strfcpy(option.value, &this->foreground_color);
                else if (strcmp(option.key, "fringe_date_color") == 0)
                    strfcpy(option.value, &this->fringe_date_color);
                else if (strcmp(option.key, "highlight_color") == 0)
                    strfcpy(option.value, &this->highlight_color);

                else if (strcmp(option.key, "month_font_size") == 0)
                    strfcpy(option.value, &this->month_font_size);
                else if (strcmp(option.key, "month_font_weight") == 0)
                    strfcpy(option.value, &this->month_font_weight);

                else if (strcmp(option.key, "day_font_size") == 0)
                    strfcpy(option.value, &this->day_font_size);
                else if (strcmp(option.key, "day_font_weight") == 0)
                    strfcpy(option.value, &this->day_font_weight);

                else if (strcmp(option.key, "date_font_size") == 0)
                    strfcpy(option.value, &this->date_font_size);
                else if (strcmp(option.key, "date_font_weight") == 0)
                    strfcpy(option.value, &this->date_font_weight);

                else if (strcmp(option.key, "arrow_font_size") == 0)
                    strfcpy(option.value, &this->arrow_font_size);
                else if (strcmp(option.key, "arrow_font_weight") == 0)
                    strfcpy(option.value, &this->arrow_font_weight);

                else if (strcmp(option.key, "position") == 0) {
                    if (strcmp(option.value, "center") == 0)
                        this->position = GTK_WIN_POS_CENTER;
                    else if (strcmp(option.value, "mouse") == 0)
                        this->position = GTK_WIN_POS_MOUSE;
                    else
                        this->position = GTK_WIN_POS_NONE;
                } else {
                    printf("Unrecognised option in config file: %s = %s\n", option.key, option.value);
                }
                free_option(option);
            }
            free(config);
        } else {
            puts("No config file loaded. Using default settings");
        }
        this->window = init_widgets(this);
        update_calendar(this);
        set_style(this);
        show_calendar(this);
    }

    return this;
}


/*
 * Free all memory allocated by the calendar instance
 */
void destroy_calendar(CalendarPtr this)
{
    free(this->month_font_size);
    free(this->month_font_weight);
    free(this->day_font_size);
    free(this->day_font_weight);
    free(this->date_font_size);
    free(this->date_font_weight);
    free(this->arrow_font_size);
    free(this->arrow_font_weight);
    free(this->background_color);
    free(this->foreground_color);
    free(this->fringe_date_color);
    free(this->highlight_color);
    if (gtk_main_level() !=0 && this->window != NULL) {
        g_object_unref(this->drawing_area);
        g_object_unref(this->window);
    }
    free(this);
}

