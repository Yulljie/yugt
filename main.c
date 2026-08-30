#include <gtk/gtk.h>
#include <vte/vte.h>

static void screen_changed(GtkWidget *widget, GdkScreen *old, gpointer data) {
    GdkScreen *screen = gtk_widget_get_screen(widget);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (!visual) visual = gdk_screen_get_system_visual(screen);
    gtk_widget_set_visual(widget, visual);
}

static gboolean on_vte_draw_after(GtkWidget *widget, cairo_t *cr, gpointer data) {
    GtkWidget *toplevel = gtk_widget_get_toplevel(widget);
    GtkStyleContext *ctx = gtk_widget_get_style_context(toplevel);
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);

    cairo_save(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_DEST_OVER);
    gtk_render_background(ctx, cr, 0, 0, w, h);
    cairo_restore(cr);
    return FALSE;
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(window), "My Terminal");
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 600);

    VteTerminal *terminal = VTE_TERMINAL(vte_terminal_new());

    const char *shell = g_getenv("SHELL");
    if (!shell || !*shell) shell = "/bin/sh";
    char *argv[] = { (char *)shell, NULL };

    vte_terminal_spawn_async(terminal, VTE_PTY_DEFAULT,
        NULL, argv, NULL, G_SPAWN_DEFAULT,
        NULL, NULL, NULL, -1, NULL, NULL, NULL);

    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(terminal));

    GdkRGBA transparent = { 0, 0, 0, 0 };
    vte_terminal_set_color_background(terminal, &transparent);

    g_signal_connect(window, "screen-changed", G_CALLBACK(screen_changed), NULL);
    screen_changed(window, NULL, NULL);

    g_signal_connect_after(GTK_WIDGET(terminal), "draw", 
    G_CALLBACK(on_vte_draw_after), NULL);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new(
        "moe.yulliil.yugt", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
