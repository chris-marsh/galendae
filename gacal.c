#include <gtk/gtk.h>

enum DaysOfWeek {
    SUNDAY = 0,
    MONDAY,
    TUESDAY,
    WEDNESDAY,
    THURSDAY,
    FRIDAY,
    SATURDAY
};

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
    int num_days;
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

struct Date {
    int day;
    int month;
    int year;
} highlight_date = { 0, 0, 0 };

struct Options {
    int show_number_of_months;
    int month_to_start_year;
    enum DaysOfWeek weekday_start;
    int month;
    int year;
} options = { 1, 1, MONDAY, 0, 0 };

GtkWidget *calendar_window;


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


void set_calendar() {
    GtkWidget *grid;
    GtkWidget *label;
    char temp[20];
    int line = 2, col;
    int first_day;
    col = 0;

    grid = gtk_bin_get_child(GTK_BIN(calendar_window));

    label = gtk_grid_get_child_at(GTK_GRID(grid), 1,0);
    sprintf(temp, "%s %d", months[options.month-1].longName, options.year);
    gtk_label_set_text(GTK_LABEL(label), temp);

    if ((options.month == 2) &&
            ((!(options.year % 4) && (options.year % 100)) || !(options.year % 400))) {
        months[options.month-1].num_days = 29;
    }
    first_day = (first_day_of_month(options.month, options.year) + options.weekday_start) % 7 - 2;

    if (grid != NULL) {
        for (int day = 1-first_day; day<=35-first_day; day++) {
            if (day<1 || day>months[options.month-1].num_days)
                temp[0] = '\0';
            else 
                sprintf(temp, "%d", day);

            label = gtk_grid_get_child_at(GTK_GRID(grid), col, line);
            gtk_label_set_text(GTK_LABEL(label), temp);
            if (day == highlight_date.day &&
                    options.month == highlight_date.month &&
                    options.year == highlight_date.year)
                gtk_widget_set_name(GTK_WIDGET(label), "highlight_day");
            else
                gtk_widget_set_name(GTK_WIDGET(label), "normal_day");


            if (++col== 7) {
                line++;
                col = 0;
            }
        }
    }
}


static void on_key_press (GtkWidget *window, GdkEventKey *eventkey, gpointer data) {
    switch (eventkey->keyval) {
        case GDK_KEY_Escape:
        case GDK_KEY_q:
            gtk_widget_destroy(window);
            break;
        case GDK_KEY_Left:
            inc_month(-1, &options.month, &options.year);
            set_calendar();
            break;
        case GDK_KEY_Right:
            inc_month(1, &options.month, &options.year);
            set_calendar();
            break;
        case GDK_KEY_Up:
            options.year++;
            set_calendar();
            break;
        case GDK_KEY_Down:
            options.year--;
            set_calendar();
            break;
    }
}


gboolean draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data) {
    GdkRGBA color;
    GtkStyleContext *context;
    int cell_width = gtk_widget_get_allocated_width(GTK_WIDGET(widget)) / 7; 
    int cell_height = gtk_widget_get_allocated_height(GTK_WIDGET(widget)) / 5; 
    
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_get_color (context, gtk_style_context_get_state (context), &color);
    gdk_cairo_set_source_rgba (cr, &color);

    cairo_arc (cr, (3*cell_width)-(cell_width/2),
            (5*cell_height)-(cell_height/2),
            (cell_height/2) - 1,
            0, 2 * G_PI);

    cairo_stroke (cr);

    return FALSE;
}


GtkWidget* init_app() {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *icon;
    GtkWidget *drawing_area;
    GtkCssProvider *provider;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(GTK_WINDOW(window), "key_release_event", G_CALLBACK(on_key_press), NULL);
    g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_set_name(GTK_WIDGET(window), "mainWindow");
    gtk_window_set_title(GTK_WINDOW(window), "Calendar");
    gtk_window_set_type_hint(GTK_WINDOW(window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_widget_set_name(GTK_WIDGET(grid), "grid");
    gtk_container_add(GTK_CONTAINER(window), grid);

    icon = gtk_image_new_from_file("images/left24a.png");
    button = gtk_button_new();
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
    gtk_button_set_image(GTK_BUTTON(button), icon);
    gtk_grid_attach(GTK_GRID(grid), button, 0,0,1,1);
    label = gtk_label_new("Month Year");
    gtk_widget_set_name(GTK_WIDGET(label), "monthLabel");
    gtk_grid_attach(GTK_GRID(grid), label, 1,0,5,1);
    icon = gtk_image_new_from_file("images/right24a.png");
    gtk_grid_attach(GTK_GRID(grid), icon, 6,0,1,1);

    for (int day =0; day<7; day++) {
        label = gtk_label_new(weekdays[(day+options.weekday_start) % 7].shortname);
        gtk_widget_set_name(GTK_WIDGET(label), "weekdayLabel");
        gtk_widget_set_margin_top(GTK_WIDGET(label), 15);
        gtk_widget_set_margin_bottom(GTK_WIDGET(label), 15);
        gtk_grid_attach(GTK_GRID(grid), label, day,1,1,1);
    }

    drawing_area = gtk_drawing_area_new ();
    g_signal_connect (G_OBJECT (drawing_area), "draw",
            G_CALLBACK (draw_callback), NULL);

    gtk_grid_attach(GTK_GRID(grid), drawing_area, 0, 2, 7, 5);
    
    for (int row = 2; row<7; row++) {
        for (int column=0; column<7; column++) {
            label = gtk_label_new("00");
            gtk_widget_set_margin_bottom(GTK_WIDGET(label),6);
            gtk_widget_set_margin_top(GTK_WIDGET(label),6);
            gtk_widget_set_margin_start(GTK_WIDGET(label),12);
            gtk_widget_set_margin_end(GTK_WIDGET(label),12);
            gtk_grid_attach(GTK_GRID(grid), label, column, row, 1, 1);
        }
    }

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_widget_show_all(window);
    return window;
}


int main(int argc, char *argv[]) {
    time_t t = time(0);
    struct tm *now = localtime(&t);
    highlight_date.day= now->tm_mday;
    highlight_date.month = now->tm_mon+1;
    highlight_date.year = now->tm_year+1900;

    options.month = highlight_date.month;
    options.year = highlight_date.year;

    gtk_init(&argc, &argv);
    calendar_window = init_app();
    set_calendar();
    gtk_main();
    return 0;
}
