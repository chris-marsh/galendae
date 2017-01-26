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


#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "gui.h"
#include "config.h"
#include "common.h"


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
    {"April", 31},
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


static void update_calendar(CalendarPtr cal)
{
    GtkWidget *grid;
    GtkWidget *label;
    char temp[20];
    unsigned int day;

    grid = gtk_bin_get_child(GTK_BIN(cal->window));
    label = gtk_grid_get_child_at(GTK_GRID(grid), 1,0);

    sprintf(temp, "%s %d", months[cal->month-1].longName, cal->year);
    gtk_label_set_text(GTK_LABEL(label), temp);

    if ((cal->month == 2) &&
            ((!(cal->year % 4) && (cal->year % 100)) || !(cal->year % 400))) {
        months[cal->month-1].num_days = 29;
    }

    if (grid != NULL) {
        day = 1 - (first_day_of_month(cal->month, cal->year) -
                cal->week_start + 7) % 7;
        for (int line = 2; line <=7; line++) {
            for (int col=0; col<7; col++) {
                if (day>0 && day<=months[cal->month-1].num_days) 
                    sprintf(temp, "%d", day);
                else
                    sprintf(temp, "%c", '\0');

                label = gtk_grid_get_child_at(GTK_GRID(grid), col, line);
                gtk_label_set_text(GTK_LABEL(label), temp);
                if (day == cal->highlight_date.day &&
                        cal->month == cal->highlight_date.month &&
                        cal->year == cal->highlight_date.year) {
                    cal->highlight_cell.column = col+1;
                    cal->highlight_cell.row = line-1;
                }
                day++;
            }
        }
    }
    gtk_widget_queue_draw_area(cal->drawing_area, 0, 0,
            gtk_widget_get_allocated_width(GTK_WIDGET(cal->drawing_area)),
            gtk_widget_get_allocated_height(GTK_WIDGET(cal->drawing_area)));
}


static void on_arrow_hover(GtkWidget *UNUSED(eventbox), GdkEvent *event, CalendarPtr cal)
{
    GdkCursor *cursor;
    GdkWindow *window;

    if (event->type == GDK_ENTER_NOTIFY) {
        cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_HAND2);
    } else {
        cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_ARROW);
    }

    window = gtk_widget_get_window(GTK_WIDGET(cal->window));
    gdk_window_set_cursor(window, cursor);
}


static void on_arrow_click(GtkWidget *eventbox, GdkEvent *UNUSED(event), CalendarPtr cal)
{
    int success = 0;
    if (gtk_widget_get_name(GTK_WIDGET(eventbox))[0] == 'l') {
        success = inc_month(-1, &cal->month, &cal->year);
    } else {
        success = inc_month(1, &cal->month, &cal->year);
    }
    if (success) update_calendar(cal);
}


static void on_key_press (GtkWidget *window, GdkEventKey *eventkey, CalendarPtr cal)
{
    switch (eventkey->keyval) {
        case GDK_KEY_q:
        case GDK_KEY_Escape:
            gtk_widget_destroy(window);
            break;
        case GDK_KEY_h:
        case GDK_KEY_Left:
            if (inc_month(-1, &cal->month, &cal->year))
                update_calendar(cal);
            break;
        case GDK_KEY_l:
        case GDK_KEY_Right:
            if (inc_month(1, &cal->month, &cal->year))
                update_calendar(cal);
            break;
        case GDK_KEY_k:
        case GDK_KEY_Up:
            if (cal->year < MAX_YEAR) {
                cal->year++;
                update_calendar(cal);
            }
            break;
        case GDK_KEY_j:
        case GDK_KEY_Down:
            if (cal->year > 1) {
                cal->year--;
                update_calendar(cal);
            }
            break;
        case GDK_KEY_g:
        case GDK_KEY_Home:
            cal->month = cal->highlight_date.month;
            cal->year = cal->highlight_date.year;
            update_calendar(cal);
    }
}


static gboolean draw_callback (GtkWidget *widget, cairo_t *cr, CalendarPtr cal)
{
    GdkRGBA color;
    GtkStyleContext *context;
    int cell_width = gtk_widget_get_allocated_width(GTK_WIDGET(widget)) / 7;
    int cell_height = gtk_widget_get_allocated_height(GTK_WIDGET(widget)) / 5;

    context = gtk_widget_get_style_context (widget);
    gtk_style_context_get_color (context, gtk_style_context_get_state (context), &color);
    gdk_cairo_set_source_rgba (cr, &color);

    if (cal->month == cal->highlight_date.month &&
            cal->year == cal->highlight_date.year) {
        cairo_arc (cr,
                (cal->highlight_cell.column*cell_width)-(cell_width/2),
                (cal->highlight_cell.row*cell_height)-(cell_height/2) - 1,
                (cell_height/2) - 1,
                0, 2 * G_PI);
    }

    cairo_stroke (cr);
    return FALSE;
}


GtkWidget* init_widgets(CalendarPtr cal)
{
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *eventbox;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Calendar");
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_stick(GTK_WINDOW(window));

    /* gtk_container_set_border_width (GTK_CONTAINER (window), 0); */
    gtk_widget_set_name(GTK_WIDGET(window), "mainWindow");
    g_signal_connect(GTK_WINDOW(window), "key_press_event", G_CALLBACK(on_key_press), cal);
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    if (cal->close_on_unfocus == 1)
        g_signal_connect(GTK_WINDOW(window),
                "focus_out_event", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_widget_set_name(GTK_WIDGET(grid), "grid");
    gtk_container_add(GTK_CONTAINER(window), grid);

    eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(eventbox), "left_eventbox");
    g_signal_connect(GTK_WIDGET(eventbox), "enter-notify-event", G_CALLBACK(on_arrow_hover), cal);
    g_signal_connect(GTK_WIDGET(eventbox), "leave-notify-event", G_CALLBACK(on_arrow_hover), cal);
    g_signal_connect(GTK_WIDGET(eventbox), "button-release-event", G_CALLBACK(on_arrow_click), cal);
    gtk_grid_attach(GTK_GRID(grid), eventbox, 0,0,1,1);

    label = gtk_label_new("<");
    gtk_widget_set_name(GTK_WIDGET(label), "leftArrow");
    gtk_grid_attach(GTK_GRID(grid), label, 0,0,1,1);

    label = gtk_label_new("Month Year");
    gtk_widget_set_name(GTK_WIDGET(label), "monthLabel");
    gtk_grid_attach(GTK_GRID(grid), label, 1,0,5,1);

    eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(eventbox), "right_eventbox");
    g_signal_connect(GTK_WIDGET(eventbox), "enter-notify-event", G_CALLBACK(on_arrow_hover), cal);
    g_signal_connect(GTK_WIDGET(eventbox), "leave-notify-event", G_CALLBACK(on_arrow_hover), cal);
    g_signal_connect(GTK_WIDGET(eventbox), "button-release-event", G_CALLBACK(on_arrow_click), cal);
    gtk_grid_attach(GTK_GRID(grid), eventbox, 6,0,1,1);

    label = gtk_label_new(">");
    gtk_widget_set_name(GTK_WIDGET(label), "rightArrow");
    gtk_grid_attach(GTK_GRID(grid), label, 6,0,1,1);

    for (int day =0; day<7; day++) {
        label = gtk_label_new(weekdays[(day+cal->week_start) % 7].shortname);
        gtk_widget_set_name(GTK_WIDGET(label), "weekdayLabel");
        gtk_widget_set_margin_top(GTK_WIDGET(label), 15);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 15);
        gtk_grid_attach(GTK_GRID(grid), label, day,1,1,1);
    }

    cal->drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_name(GTK_WIDGET(cal->drawing_area), "drawArea");
    g_signal_connect (G_OBJECT (cal->drawing_area), "draw", G_CALLBACK (draw_callback), cal);
    gtk_grid_attach(GTK_GRID(grid), cal->drawing_area, 0, 2, 7, 5);

    for (int row = 2; row<8; row++) {
        for (int column=0; column<7; column++) {
            label = gtk_label_new(NULL);
            gtk_widget_set_margin_bottom(GTK_WIDGET(label),5);
            gtk_widget_set_margin_top(GTK_WIDGET(label),5);
            gtk_widget_set_margin_start(GTK_WIDGET(label),11);
            gtk_widget_set_margin_end(GTK_WIDGET(label),11);
            gtk_grid_attach(GTK_GRID(grid), label, column, row, 1, 1);
        }
    }

    return window;
}


void set_style(CalendarPtr cal)
{
    char style_data[2048] = {0}; // TODO calculate actual space needed
    sprintf(style_data,
            "#mainWindow {background-color:%s; color:%s; font-size:%s; font-weight:%s;}"
            "#monthLabel {font-size:%s; font-weight:%s;}"
            "#weekdayLabel {font-size:%s; font-weight:%s;}"
            "#leftArrow, #rightArrow {font-size:%s; font-weight:%s;}",
            cal->background_color, cal->foreground_color,
            cal->date_font_size, cal->date_font_weight,
            cal->month_font_size, cal->month_font_weight,
            cal->day_font_size, cal->day_font_weight,
            cal->arrow_font_size, cal->arrow_font_weight);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(GTK_CSS_PROVIDER(provider),
            style_data, strlen(style_data), NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
}


static void show_calendar(CalendarPtr cal)
{
    gint x_pos = 0, y_pos = 0;

    gtk_window_set_position(GTK_WINDOW(cal->window), cal->position);
    gtk_window_get_position(GTK_WINDOW(cal->window), &x_pos, &y_pos);
    gtk_window_move(GTK_WINDOW(cal->window), cal->x_offset + x_pos,  cal->y_offset + y_pos);

    gtk_widget_show_all(cal->window);
}


void set_default_config(CalendarPtr cal)
{
    cal->highlight_date = today();
    cal->month = cal->highlight_date.month;
    cal->year = cal->highlight_date.year;
    cal->week_start = MONDAY;

    cal->close_on_unfocus = 0;
    cal->position = GTK_WIN_POS_NONE;
    cal->x_offset = 0;
    cal->y_offset = 0;

    cal->background_color = strdup("#afafaf");
    cal->foreground_color = strdup("#000000");

    cal->month_font_size = strdup("1.0em");
    cal->month_font_weight = strdup("normal");

    cal->day_font_size = strdup("1.0em");
    cal->day_font_weight = strdup("normal");

    cal->date_font_size = strdup("1.0em");
    cal->date_font_weight = strdup("normal");

    cal->arrow_font_size = strdup("1.0em");
    cal->arrow_font_weight = strdup("normal");
}


CalendarPtr create_calendar(char *config_filename)
{
    CalendarPtr cal = malloc(sizeof *cal);

    if (cal) {
        set_default_config(cal);

        Config *config = read_config_file(config_filename);
        if (config) {
            Option option;
            while (config != NULL) {
                option = pop_option(&config);

                if (strcmp(option.key, "close_on_unfocus") == 0)
                    cal->close_on_unfocus = atoi(option.value);

                if (strcmp(option.key, "position") == 0) {
                    if (strcmp(option.value, "center") == 0) {
                        cal->position = GTK_WIN_POS_CENTER;
                    } else if (strcmp(option.value, "mouse") == 0) {
                        cal->position = GTK_WIN_POS_MOUSE;
                    } else {
                        cal->position = GTK_WIN_POS_NONE;
                    }
                }

                if (strcmp(option.key, "x_offset") == 0)
                    cal->x_offset = atoi(option.value);
                if (strcmp(option.key, "y_offset") == 0)
                    cal->y_offset = atoi(option.value);

                if (strcmp(option.key, "background_color") == 0)
                    strfcpy(option.value, &cal->background_color);
                if (strcmp(option.key, "foreground_color") == 0)
                    strfcpy(option.value, &cal->foreground_color);

                if (strcmp(option.key, "month_font_size") == 0){
                    strfcpy(option.value, &cal->month_font_size);
                }
                if (strcmp(option.key, "month_font_weight") == 0)
                    strfcpy(option.value, &cal->month_font_weight);

                if (strcmp(option.key, "day_font_size") == 0)
                    strfcpy(option.value, &cal->day_font_size);
                if (strcmp(option.key, "day_font_weight") == 0)
                    strfcpy(option.value, &cal->day_font_weight);

                if (strcmp(option.key, "date_font_size") == 0)
                    strfcpy(option.value, &cal->date_font_size);
                if (strcmp(option.key, "day_font_weight") == 0)
                    strfcpy(option.value, &cal->date_font_weight);

                if (strcmp(option.key, "arrow_font_size") == 0)
                    strfcpy(option.value, &cal->arrow_font_size);
                if (strcmp(option.key, "arrow_font_weight") == 0)
                    strfcpy(option.value, &cal->arrow_font_weight);

                free_option(option);
            }
            free(config);
        } else {
            puts("No config file loaded. Using default settings");
        }
        cal->window = init_widgets(cal);
        update_calendar(cal);
        set_style(cal);
        show_calendar(cal);
    }

    return cal;
}

void destroy_calendar(CalendarPtr cal)
{
    free(cal->month_font_size);
    free(cal->month_font_weight);
    free(cal->day_font_size);
    free(cal->day_font_weight);
    free(cal->date_font_size);
    free(cal->date_font_weight);
    free(cal->arrow_font_size);
    free(cal->arrow_font_weight);
    free(cal->background_color);
    free(cal->foreground_color);
    if (gtk_main_level() !=0 && cal->window != NULL) {
        g_object_unref(cal->drawing_area);
        g_object_unref(cal->window);
        /* gtk_widget_destroy(cal->drawing_area); */
        /* gtk_widget_destroy(cal->window); */
    }
    free(cal);
}

