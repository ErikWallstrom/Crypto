#include <gtk/gtk.h>
#include <stdlib.h>
#include "crypto.h"
#include "log.h"

void onerror(void* userdata)
{
	(void)userdata;
	abort();
}

void about_action(GSimpleAction* action, GVariant* param, void* userdata)
{
	(void)action;
	(void)param;

	GtkWindow* window = gtk_application_get_active_window(
		GTK_APPLICATION(userdata)
	);

	gtk_show_about_dialog(
		window,
		"program-name", "Crypto",
		"copyright", "Copyright © 2018 Erik Wallström",
		"authors", (char*[]){"Erik Wallström", NULL},
		"documenters", (char*[]){"Erik Wallström", NULL},
		"website", "https://my_website.com/",
		"website_label", "My Website",
		"version", "0.1 Alpha",
		"comments", "GTK3 Wrapper for CoinCap.io",
		//"logo", gdk_pixbuf_new_from_file("gaben.png", NULL),
		NULL
	);
}

void quit_action(GSimpleAction* action, GVariant* param, void* userdata)
{
	(void)action;
	(void)param;

	GtkWindow* window = gtk_application_get_active_window(
		GTK_APPLICATION(userdata)
	);

	gtk_widget_destroy(GTK_WIDGET(window));
	g_application_quit(G_APPLICATION(userdata));
}

void app_init(GtkApplication* app, void* userdata)
{
	(void)userdata;

	GActionEntry appactions[] = {
		{"about", about_action, NULL, NULL, NULL, {0}},
		{"quit", quit_action, NULL, NULL, NULL, {0}},
	};

	g_action_map_add_action_entries(
		G_ACTION_MAP(app),
		appactions,
		sizeof appactions / sizeof *appactions,
		app
	);

	GMenu* menu = g_menu_new();
	g_menu_append(menu, "About", "app.about");
	g_menu_append(menu, "Quit", "app.quit");

	gtk_application_set_app_menu(app, G_MENU_MODEL(menu));
	gtk_application_set_accels_for_action(
        app,
        "app.quit",
        (const char*[]){"<Ctrl>Q", NULL}
    );
}

void app_ctor(GtkApplication* app, void* userdata)
{
	(void)userdata;

	GtkWidget* window = gtk_application_window_new(app);
	GtkWidget* headerbar = gtk_header_bar_new();
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerbar), 1);
	gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 800);

	crypto_init(window);
	gtk_widget_show_all(window);
}

int main(int argc, char* argv[])
{
	log_seterrorhandler(onerror, NULL);

	GtkApplication* app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "startup", G_CALLBACK(app_init), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(app_ctor), NULL);
	
	int ret = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	return ret;
}

