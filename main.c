#include <stdlib.h>
#include <gtk/gtk.h>
#include <curl/curl.h>
#include <json-glib/json-glib.h>
#include "string.h"
#include "str.h"
#include "log.h"

void onerror(void* userdata)
{
	(void)userdata;
	abort();
}

size_t writefunc(char* ptr, size_t size, size_t nummembers, void* userdata)
{
	size_t len = size * nummembers;
	struct Str* str = userdata;
	str_appendfmt(str, "%.*s", (int)len, ptr);
	return len;
}

struct Str http_get(const char* url)
{
	struct Str str;
	str_ctor(&str, "");

	CURL* curl = curl_easy_init();
	if(!curl)
	{
		log_error("Curl failed to initialize");
	}

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);

	CURLcode result = curl_easy_perform(curl);
	if(result)
	{
		log_error("%s", curl_easy_strerror(result));
	}

	curl_easy_cleanup(curl);
	return str;
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

enum Column
{
	COLUMN_NAME,
	COLUMN_MARKETCAP,
	COLUMN_PRICE,
	COLUMN_CHANGE,
};

void name_cell_data(
	GtkTreeViewColumn* column, 
	GtkCellRenderer* renderer,
	GtkTreeModel* model,
	GtkTreeIter* iter,
	void* userdata)
{
	(void)column;
	(void)model;
	(void)iter;
	(void)userdata;

	g_object_set(renderer, "foreground-rgba", NULL, NULL);
}

void change_cell_data(
	GtkTreeViewColumn* column, 
	GtkCellRenderer* renderer,
	GtkTreeModel* model,
	GtkTreeIter* iter,
	void* userdata)
{
	(void)column;
	(void)model;
	(void)iter;
	(void)userdata;

	gchar* text;
	g_object_get(renderer, "text", &text, NULL);
	if(text[0] == '+')
	{
		GdkRGBA color = {0.0, 1.0, 0.0, 1.0};
		g_object_set(renderer, "foreground-rgba", &color, NULL);
	}
	else
	{
		GdkRGBA color = {1.0, 0.0, 0.0, 1.0};
		g_object_set(renderer, "foreground-rgba", &color, NULL);
	}
}

void app_ctor(GtkApplication* app, void* userdata)
{
	(void)userdata;

	GtkWidget* window = gtk_application_window_new(app);
	GtkWidget* headerbar = gtk_header_bar_new();
	gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerbar), 1);
	gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
	gtk_window_set_default_size(GTK_WINDOW(window), 600, 800);
	
	GtkListStore* store = gtk_list_store_new(
		4, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING,
		G_TYPE_STRING
	);

	struct Str str = http_get("coincap.io/front");
	GError* error = NULL;
	JsonNode* root = json_from_string(str.data, &error);
	if(error)
	{
		log_error("%s", error->message);
		g_error_free(error);
	}

	JsonArray* array = json_node_get_array(root);
	guint len = json_array_get_length(array);
	for(guint i = 0; i < ((len > 40) ? 40 : len); i++)
	{
		JsonNode* node = json_array_get_element(array, i);
		JsonObject* object = json_node_get_object(node);
		const gchar* name = json_object_get_string_member(object, "long");
		double marketcap = json_object_get_double_member(object, "mktcap");
		double price = json_object_get_double_member(object, "price");
		double change = json_object_get_double_member(object, "cap24hrChange");

		struct Str marketcapfmt;
		str_ctorfmt(&marketcapfmt, "%li", (glong)marketcap);
		if(marketcapfmt.len > 3)
		{
			size_t j = marketcapfmt.len - 3;
			while(j >= 3)
			{
				str_insertfmt(&marketcapfmt, j, "%c", ',');
				j -= 3;
			}

			if(j > 0)
			{
				str_insertfmt(&marketcapfmt, j, "%c", ',');
			}
		}

		struct Str pricefmt;
		str_ctorfmt(&pricefmt, "%g", price);

		struct Str changefmt;
		str_ctorfmt(&changefmt, "%g", change);
		if(changefmt.data[0] != '-')
		{
			str_prependfmt(&changefmt, "%c", '+');
		}

		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(
			store, 
			&iter, 
			COLUMN_NAME, name, 
			COLUMN_MARKETCAP, marketcapfmt.data,
			COLUMN_PRICE, pricefmt.data,
			COLUMN_CHANGE, changefmt.data,
			-1
		);

		str_dtor(&changefmt);
		str_dtor(&pricefmt);
		str_dtor(&marketcapfmt);
	}

	json_node_free(root); //Does this free everything?
	str_dtor(&str);

	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "font", "Monospace 14", NULL);

	GtkWidget* tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
		"Name",
		renderer,
		"text", COLUMN_NAME,
		NULL
	);

	gtk_tree_view_column_set_cell_data_func(
		column, 
		renderer, 
		name_cell_data,
		NULL,
		NULL
	);

	gtk_tree_view_column_set_expand(column, 1);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes(
		"Market Cap",
		renderer,
		"text", COLUMN_MARKETCAP,
		NULL
	);

	gtk_tree_view_column_set_expand(column, 1);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes(
		"Price",
		renderer,
		"text", COLUMN_PRICE,
		NULL
	);

	gtk_tree_view_column_set_expand(column, 1);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes(
		"%24hr",
		renderer,
		"text", COLUMN_CHANGE,
		NULL
	);

	gtk_tree_view_column_set_cell_data_func(
		column, 
		renderer, 
		change_cell_data,
		NULL,
		NULL
	);

	gtk_tree_view_column_set_expand(column, 1);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

	GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(box), tree);
	gtk_container_add(GTK_CONTAINER(scrolled), box);
	gtk_container_add(GTK_CONTAINER(window), scrolled);

	gtk_widget_show_all(window);
}

int main(int argc, char* argv[])
{
	log_seterrorhandler(onerror, NULL);

	GtkApplication* app = gtk_application_new(NULL, G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "startup", G_CALLBACK(app_init), NULL);
	g_signal_connect(app, "activate", G_CALLBACK(app_ctor), NULL);

	return g_application_run(G_APPLICATION(app), argc, argv);
}

