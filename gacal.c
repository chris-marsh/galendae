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
} static weekdays[] = {
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
    int num_days;
} static months[] = {
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


struct Point {
    int row;
    int column;
};


struct App {
    GtkWidget *window;
    int month;
    int year;
    enum Days_of_Week weekday_start;
    struct Date highlight_date;
    struct Point highlight_cell;
} static app = { NULL, 0, 0, MONDAY, { 0, 0, 0}, { 0, 0 } };


/*
 * Return struct with current system date
 */
struct Date today(void) {
    time_t t = time(0);
    struct tm *now = localtime(&t);
    struct Date date = {
        now->tm_mday,
        now->tm_mon+1,
        now->tm_year+1900
    };
    return date;
}


/*
 * Set which day of the week will be the calendars first column
 */
void set_weekday_start(enum Days_of_Week day) {
    app.weekday_start = day;
}


/*
 * Set date to highlight on the calendar
 */
void set_highlight_date(struct Date date) {
    app.highlight_date = date;
}


void set_current_date(struct Date date) {
    app.month = date.month;
    app.year = date.year;
}


void inc_month(int num, int *month, int *year) {
    int tempYear  = *year;
    int tempMonth = *month - 1;

    *year = (tempYear * 12 + tempMonth + num) / 12;
    *month = (tempYear * 12 + tempMonth + num) % 12 + 1;
}


int first_day_of_month(int month, int year) {
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

static GtkWidget* find_child(GtkWidget* parent, const gchar* name) {
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


void set_calendar() {
    GtkWidget *grid;
    GtkWidget *label;
    char temp[20];
    int day;

    grid = gtk_bin_get_child(GTK_BIN(app.window));
    label = gtk_grid_get_child_at(GTK_GRID(grid), 1,0);

    sprintf(temp, "%s %d", months[app.month-1].longName, app.year);
    gtk_label_set_text(GTK_LABEL(label), temp);

    if ((app.month == 2) &&
            ((!(app.year % 4) && (app.year % 100)) || !(app.year % 400))) {
        months[app.month-1].num_days = 29;
    }

    if (grid != NULL) {
        day = 1 - (first_day_of_month(app.month, app.year) -
                app.weekday_start + 7) % 7;
        for (int line = 2; line <=7; line++) {
            for (int col=0; col<7; col++) {
                if (day>0 && day<=months[app.month-1].num_days) 
                    sprintf(temp, "%d", day);
                else
                    sprintf(temp, "%c", '\0');

                label = gtk_grid_get_child_at(GTK_GRID(grid), col, line);
                gtk_label_set_text(GTK_LABEL(label), temp);
                if (day == app.highlight_date.day &&
                        app.month == app.highlight_date.month &&
                        app.year == app.highlight_date.year) {
                    app.highlight_cell.column = col+1;
                    app.highlight_cell.row = line-1;
                }
                day++;
            }
        }
    }
    GtkWidget *da = find_child(app.window, "drawArea");
    gtk_widget_queue_draw_area(da, 0, 0,
            gtk_widget_get_allocated_width(GTK_WIDGET(da)),
            gtk_widget_get_allocated_height(GTK_WIDGET(da)));
}


static void on_arrow_hoover(GtkWidget *UNUSED(eventbox), GdkEvent *event) {
    GdkCursor *cursor;
    GdkWindow *window;

    if (event->type == GDK_ENTER_NOTIFY) {
        cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_HAND2);
    } else {
        cursor = gdk_cursor_new_for_display(gdk_display_get_default(), GDK_ARROW);
    }

    window = gtk_widget_get_window(GTK_WIDGET(app.window));
    gdk_window_set_cursor(window, cursor);
}


static void on_arrow_click(GtkWidget *eventbox) {
    if (gtk_widget_get_name(GTK_WIDGET(eventbox))[0] == 'l') {
        inc_month(-1, &app.month, &app.year);
    } else {
        inc_month(1, &app.month, &app.year);
    }
    set_calendar();
}


static void on_key_press (GtkWidget *window, GdkEventKey *eventkey) {
    switch (eventkey->keyval) {
        case GDK_KEY_Escape:
        case GDK_KEY_q:
            gtk_widget_destroy(window);
            break;
        case GDK_KEY_Left:
            inc_month(-1, &app.month, &app.year);
            set_calendar();
            break;
        case GDK_KEY_Right:
            inc_month(1, &app.month, &app.year);
            set_calendar();
            break;
        case GDK_KEY_Up:
            app.year++;
            set_calendar();
            break;
        case GDK_KEY_Down:
            app.year--;
            set_calendar();
            break;
    }
}


static gboolean draw_callback (GtkWidget *widget, cairo_t *cr) {
    GdkRGBA color;
    GtkStyleContext *context;
    int cell_width = gtk_widget_get_allocated_width(GTK_WIDGET(widget)) / 7;
    int cell_height = gtk_widget_get_allocated_height(GTK_WIDGET(widget)) / 5;

    context = gtk_widget_get_style_context (widget);
    gtk_style_context_get_color (context, gtk_style_context_get_state (context), &color);
    gdk_cairo_set_source_rgba (cr, &color);

    if (app.month == app.highlight_date.month &&
            app.year == app.highlight_date.year) {
        cairo_arc (cr,
                (app.highlight_cell.column*cell_width)-(cell_width/2),
                (app.highlight_cell.row*cell_height)-(cell_height/2) - 1,
                (cell_height/2) - 1,
                0, 2 * G_PI);
    }

    cairo_stroke (cr);
    return FALSE;
}


void init_app() {
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *eventbox;
    GtkWidget *icon;
    GtkWidget *drawing_area;
    GtkCssProvider *provider;

    app.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_WINDOW(app.window), "key_release_event", G_CALLBACK(on_key_press), NULL);
    g_signal_connect(GTK_WINDOW(app.window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_set_name(GTK_WIDGET(app.window), "mainWindow");
    gtk_window_set_title(GTK_WINDOW(app.window), "Calendar");
    gtk_window_set_type_hint(GTK_WINDOW(app.window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_container_set_border_width(GTK_CONTAINER(app.window), 10);

    grid = gtk_grid_new();
    gtk_widget_set_name(GTK_WIDGET(grid), "grid");
    gtk_container_add(GTK_CONTAINER(app.window), grid);

    eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(eventbox), "left_eventbox");
    g_signal_connect(GTK_WIDGET(eventbox), "enter-notify-event", G_CALLBACK(on_arrow_hoover), NULL);
    g_signal_connect(GTK_WIDGET(eventbox), "leave-notify-event", G_CALLBACK(on_arrow_hoover), NULL);
    g_signal_connect(GTK_WIDGET(eventbox), "button-release-event", G_CALLBACK(on_arrow_click), NULL);
    gtk_grid_attach(GTK_GRID(grid), eventbox, 0,0,1,1);
    icon = gtk_image_new_from_file("images/left.png");
    gtk_grid_attach(GTK_GRID(grid), icon, 0,0,1,1);

    label = gtk_label_new("Month Year");
    gtk_widget_set_name(GTK_WIDGET(label), "monthLabel");
    gtk_grid_attach(GTK_GRID(grid), label, 1,0,5,1);

    eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(eventbox), "right_eventbox");
    g_signal_connect(GTK_WIDGET(eventbox), "enter-notify-event", G_CALLBACK(on_arrow_hoover), NULL);
    g_signal_connect(GTK_WIDGET(eventbox), "leave-notify-event", G_CALLBACK(on_arrow_hoover), NULL);
    g_signal_connect(GTK_WIDGET(eventbox), "button-release-event", G_CALLBACK(on_arrow_click), NULL);
    gtk_grid_attach(GTK_GRID(grid), eventbox, 6,0,1,1);
    icon = gtk_image_new_from_file("images/right.png");
    gtk_grid_attach(GTK_GRID(grid), icon, 6,0,1,1);

    for (int day =0; day<7; day++) {
        label = gtk_label_new(weekdays[(day+app.weekday_start) % 7].shortname);
        gtk_widget_set_name(GTK_WIDGET(label), "weekdayLabel");
        gtk_widget_set_margin_top(GTK_WIDGET(label), 15);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 15);
        gtk_grid_attach(GTK_GRID(grid), label, day,1,1,1);
    }

    drawing_area = gtk_drawing_area_new ();
    gtk_widget_set_name(GTK_WIDGET(drawing_area), "drawArea");
    g_signal_connect (G_OBJECT (drawing_area), "draw", G_CALLBACK (draw_callback), NULL);
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

    gtk_widget_show_all(app.window);
}

