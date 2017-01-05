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
#include <gtk/gtk.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>


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
    unsigned int  column;
} Point;


struct Calendar {
    GtkWidget *window;
    unsigned int month;
    unsigned int year;
    Days_of_Week week_start;
    Date highlight_date;
    Point highlight_cell;
    int x_offset;
    int y_offset;
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

static GtkWidget* find_child(GtkWidget* parent, const gchar* name)
{
    if (g_ascii_strcasecmp(gtk_widget_get_name((GtkWidget*)parent), (gchar*)name) == 0) { 
        return parent;
    }

    if (GTK_IS_BIN(parent)) {
        GtkWidget *child = gtk_bin_get_child(GTK_BIN(parent));
        return find_child(child, name);
    }

    if (GTK_IS_CONTAINER(parent)) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(parent));
        while ((children = g_list_next(children)) != NULL) {
            GtkWidget* widget = find_child(children->data, name);
            if (widget != NULL) {
                return widget;
            }
        }
    }

    return NULL;
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
    GtkWidget *da = find_child(cal->window, "drawArea");
    gtk_widget_queue_draw_area(da, 0, 0,
            gtk_widget_get_allocated_width(GTK_WIDGET(da)),
            gtk_widget_get_allocated_height(GTK_WIDGET(da)));
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
    GtkWidget *icon;
    GtkWidget *drawing_area;
    GtkCssProvider *provider;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_keep_above(GTK_WINDOW(window), TRUE);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_stick(GTK_WINDOW(window));
    gtk_widget_set_name(GTK_WIDGET(window), "mainWindow");
    gtk_window_set_title(GTK_WINDOW(window), "Calendar");
    g_signal_connect(GTK_WINDOW(window), "key_release_event", G_CALLBACK(on_key_press), cal);
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(GTK_WINDOW(window), "focus_out_event", G_CALLBACK(gtk_widget_destroy), NULL);
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
    icon = gtk_image_new_from_file("images/left.png");
    gtk_grid_attach(GTK_GRID(grid), icon, 0,0,1,1);

    label = gtk_label_new("Month Year");
    gtk_widget_set_name(GTK_WIDGET(label), "monthLabel");
    gtk_grid_attach(GTK_GRID(grid), label, 1,0,5,1);

    eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(eventbox), "right_eventbox");
    g_signal_connect(GTK_WIDGET(eventbox), "enter-notify-event", G_CALLBACK(on_arrow_hover), cal);
    g_signal_connect(GTK_WIDGET(eventbox), "leave-notify-event", G_CALLBACK(on_arrow_hover), cal);
    g_signal_connect(GTK_WIDGET(eventbox), "button-release-event", G_CALLBACK(on_arrow_click), cal);
    gtk_grid_attach(GTK_GRID(grid), eventbox, 6,0,1,1);
    icon = gtk_image_new_from_file("images/right.png");
    gtk_grid_attach(GTK_GRID(grid), icon, 6,0,1,1);

    for (int day =0; day<7; day++) {
        label = gtk_label_new(weekdays[(day+cal->week_start) % 7].shortname);
        gtk_widget_set_name(GTK_WIDGET(label), "weekdayLabel");
        gtk_widget_set_margin_top(GTK_WIDGET(label), 15);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 15);
        gtk_grid_attach(GTK_GRID(grid), label, day,1,1,1);
    }

    drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_name(GTK_WIDGET(drawing_area), "drawArea");
    g_signal_connect (G_OBJECT (drawing_area), "draw", G_CALLBACK (draw_callback), cal);
    gtk_grid_attach(GTK_GRID(grid), drawing_area, 0, 2, 7, 5);

    for (int row = 2; row<8; row++) {
        for (int column=0; column<7; column++) {
            label = gtk_label_new(NULL);
            gtk_widget_set_margin_bottom(GTK_WIDGET(label),6);
            gtk_widget_set_margin_top(GTK_WIDGET(label),6);
            gtk_widget_set_margin_start(GTK_WIDGET(label),12);
            gtk_widget_set_margin_end(GTK_WIDGET(label),12);
            gtk_grid_attach(GTK_GRID(grid), label, column, row, 1, 1);
        }
    }

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
            GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    return window;
}


static void show_window(CalendarPtr cal)
{
    gint xpos, ypos;

    gtk_window_set_position(GTK_WINDOW(cal->window), GTK_WIN_POS_CENTER);
    gtk_window_get_position(GTK_WINDOW(cal->window), &xpos, &ypos);
    gtk_window_move(GTK_WINDOW(cal->window), cal->x_offset + xpos,  cal->y_offset + ypos);
    gtk_widget_show_all(cal->window);
}


CalendarPtr create_calendar(Options options)
{
    CalendarPtr cal = malloc(sizeof *cal);

    if (cal) {
        cal->month = options.month;
        cal->year = options.year;
        cal->highlight_date = options.highlight;
        cal->x_offset = options.x_offset;
        cal->y_offset = options.y_offset;
        cal->week_start = options.week_start;
        cal->window = init_widgets(cal);
        update_calendar(cal);
        show_window(cal);
    }

    return cal;
}

void destroy_calendar(CalendarPtr cal)
{
    if (gtk_main_level() !=0 && cal->window != NULL)
        gtk_widget_destroy(cal->window);
    free(cal);
}

