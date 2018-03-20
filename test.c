#include "test.h"
#include "log.h"
#include "str.h"
#include "httpclient.h"
#include <string.h>
#include <stdlib.h>
#include <json-glib/json-glib.h>

enum ColumnType
{
	COLUMNTYPE_NAME,
	COLUMNTYPE_MARKETCAP,
	COLUMNTYPE_PRICE,
	COLUMNTYPE_SUPPLY,
	COLUMNTYPE_VOLUME,
	COLUMNTYPE_CHANGE,
	NUM_COLUMNTYPES
};

static void populate_store(GtkListStore* store)
{
	log_assert(store, "is NULL");

	struct HTTPClient* http = httpclient_new("coincap.io/front");
	struct Str httpresult = httpclient_get(http);
	
	GError* error = NULL;
	JsonNode* root = json_from_string(httpresult.data, &error);
	if(error)
	{
		log_error("%s", error->message);
		g_error_free(error);
	}

	JsonArray* array = json_node_get_array(root);
	guint len = json_array_get_length(array);
	GtkTreeIter iter;
	for(size_t i = 0; i < len; i++)
	{
		JsonNode* node = json_array_get_element(array, i);
		JsonObject* object = json_node_get_object(node);
		const gchar* name = json_object_get_string_member(object, "long");
		double marketcap = json_object_get_double_member(object, "mktcap");
		double price = json_object_get_double_member(object, "price");
		long supply = json_object_get_int_member(object, "supply");
		long volume = json_object_get_int_member(object, "volume");
		double change;
		if(!json_object_get_null_member(object, "cap24hrChange"))
		{
 			change = json_object_get_double_member(object, "cap24hrChange");
		}
		else
		{
			change = 0.0;
		}

		struct Str marketcapfmt;
		str_ctorfmt(&marketcapfmt, "%'li", (glong)marketcap);

		struct Str pricefmt;
		str_ctorfmt(&pricefmt, "%g", price);

		struct Str supplyfmt;
		str_ctorfmt(&supplyfmt, "%'li", supply);

		struct Str volumefmt;
		str_ctorfmt(&volumefmt, "%'li", volume);

		struct Str changefmt;
		str_ctorfmt(&changefmt, "%g", change);
		if(changefmt.data[0] != '-')
		{
			str_prependfmt(&changefmt, "%c", '+');
		}

		gtk_list_store_append(store, &iter);
		gtk_list_store_set(
			store, 
			&iter, 
			COLUMNTYPE_NAME, name,
			COLUMNTYPE_MARKETCAP, marketcapfmt.data,
			COLUMNTYPE_PRICE, pricefmt.data,
			COLUMNTYPE_SUPPLY, supplyfmt.data,
			COLUMNTYPE_VOLUME, volumefmt.data,
			COLUMNTYPE_CHANGE, changefmt.data,
			-1
		);

		str_dtor(&changefmt);
		str_dtor(&pricefmt);
		str_dtor(&marketcapfmt);
	}

	json_node_free(root); //Does this free everything?
	str_dtor(&httpresult);
	httpclient_delete(http);
}

static void change_cell_data(
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

	g_free(text);
}

static void add_column(
	GtkTreeView* tree, 
	const char* title, 
	enum ColumnType type)
{
	log_assert(tree, "is NULL");
	log_assert(title, "is NULL");
	log_assert(type < NUM_COLUMNTYPES, "invalid type passed (%i)", type);

	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "font", "Monospace 14", NULL);

	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
		title, 
		renderer, 
		"text", type, 
		NULL
	);

	if(type == COLUMNTYPE_CHANGE)
	{
		gtk_tree_view_column_set_cell_data_func(
			column, 
			renderer, 
			change_cell_data,
			NULL,
			NULL
		);
	}
	else
	{
		gtk_tree_view_column_set_expand(column, 1);
	}

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(column, type);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(tree, column);
}

static gint marketcap_sort(
	GtkTreeModel* model, 
	GtkTreeIter* a, 
	GtkTreeIter* b, 
	void* userdata)
{
	gchar* name1;
	gchar* name2;
	gtk_tree_model_get(model, a, GPOINTER_TO_INT(userdata), &name1, -1);
	gtk_tree_model_get(model, b, GPOINTER_TO_INT(userdata), &name2, -1);

	size_t len1 = strlen(name1);
	size_t len2 = strlen(name2);

	gint ret = 0;
	if(len1 > len2)
	{
		ret = 1;
	}
	else if(len2 > len1)
	{
		ret = -1;
	}
	else
	{
		for(size_t i = 0; i < len1; i++)
		{
			if(name1[i] > name2[i])
			{
				ret = 1;
				break;
			}
			else if(name2[i] > name1[i])
			{
				ret = -1;
				break;
			}
		}
	}

	g_free(name1);
	g_free(name2);
	return ret;
}

static gint price_sort(
	GtkTreeModel* model, 
	GtkTreeIter* a, 
	GtkTreeIter* b, 
	void* userdata)
{
	(void)userdata;

	gchar* name1;
	gchar* name2;
	gtk_tree_model_get(model, a, COLUMNTYPE_PRICE, &name1, -1);
	gtk_tree_model_get(model, b, COLUMNTYPE_PRICE, &name2, -1);

	double price1 = atof(name1);
	double price2 = atof(name2);

	gint ret = 0;
	if(price1 > price2)
	{
		ret = 1;
	}
	else if(price2 > price1)
	{
		ret = -1;
	}

	g_free(name1);
	g_free(name2);
	return ret;
}

static gint change_sort(
	GtkTreeModel* model, 
	GtkTreeIter* a, 
	GtkTreeIter* b, 
	void* userdata)
{
	(void)userdata;

	gchar* name1;
	gchar* name2;
	gtk_tree_model_get(model, a, COLUMNTYPE_CHANGE, &name1, -1);
	gtk_tree_model_get(model, b, COLUMNTYPE_CHANGE, &name2, -1);

	double price1 = atof(name1);
	double price2 = atof(name2);

	gint ret = 0;
	if(price1 > price2)
	{
		ret = 1;
	}
	else if(price2 > price1)
	{
		ret = -1;
	}

	g_free(name1);
	g_free(name2);
	return ret;
}

GtkWidget* frontpage_new(void)
{
	GtkListStore* store = gtk_list_store_new(
		NUM_COLUMNTYPES, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING
	);

	GtkTreeSortable* sortable = GTK_TREE_SORTABLE(store);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMNTYPE_MARKETCAP, 
		marketcap_sort, 
		GINT_TO_POINTER(COLUMNTYPE_MARKETCAP), 
		NULL
	);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMNTYPE_PRICE, 
		price_sort, 
		NULL, 
		NULL
	);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMNTYPE_SUPPLY, 
		marketcap_sort, 
		GINT_TO_POINTER(COLUMNTYPE_SUPPLY), 
		NULL
	);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMNTYPE_VOLUME, 
		marketcap_sort, 
		GINT_TO_POINTER(COLUMNTYPE_VOLUME), 
		NULL
	);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMNTYPE_CHANGE, 
		change_sort, 
		NULL, 
		NULL
	);
	populate_store(store);

	GtkWidget* tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(tree), 1);
	gtk_tree_view_set_enable_search(GTK_TREE_VIEW(tree), 1);
	gtk_tree_view_set_grid_lines(
		GTK_TREE_VIEW(tree), 
		GTK_TREE_VIEW_GRID_LINES_BOTH
	);

	const char* titles[] = {
		"Name", 
		"Market Cap", 
		"Price", 
		"Supply", 
		"Volume", 
		"Change", 
	};

	for(size_t i = 0; i < NUM_COLUMNTYPES; i++)
	{
		add_column(GTK_TREE_VIEW(tree), titles[i], i);
	}

	GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled), tree);
	g_object_unref(G_OBJECT(store));

	return scrolled;
}

GtkWidget* globalstats_new(void)
{
	struct HTTPClient* http = httpclient_new("coincap.io/global");
	struct Str httpresult = httpclient_get(http);

	GError* error = NULL;
	JsonNode* root = json_from_string(httpresult.data, &error);
	if(error)
	{
		log_error("%s", error->message);
		g_error_free(error);
	}

	JsonObject* object = json_node_get_object(root);
	double totmarketcap = json_object_get_double_member(object, "totalCap");
	double bitmarketcap = json_object_get_double_member(object, "btcCap");
	double altmarketcap = json_object_get_double_member(object, "altCap");
	double totvolume = json_object_get_double_member(object, "volumeTotal");
	double bitvolume = json_object_get_double_member(object, "volumeBtc");
	double altvolume = json_object_get_double_member(object, "volumeAlt");
	double bitprice = json_object_get_double_member(object, "btcPrice");
	double bitcapdom = bitmarketcap / totmarketcap * 100.0;
	double bitvolumedom = bitvolume / totvolume * 100.0;

	GtkWidget* grid = gtk_grid_new();
	gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
	gtk_grid_set_column_homogeneous(GTK_GRID(grid), 1);

	GtkWidget* label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"36\" alpha=\"50%\">Total Cap</span>"
	);
	//gtk_widget_set_vexpand(label, 1);
	gtk_widget_set_valign(label, GTK_ALIGN_CENTER);
	gtk_widget_set_size_request(label, -1, 160);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

	label = gtk_label_new(NULL);
	struct Str str;
	str_ctorfmt(
		&str, 
		"<span font=\"36\"><tt>%'li</tt></span>", 
		(long)totmarketcap
	);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
	str_dtor(&str);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Bitcoin Cap</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(
		&str, 
		"<span font=\"18\"><tt>%'li</tt></span>", 
		(long)bitmarketcap
	);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 1, 1, 1);
	str_dtor(&str);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Altcoin Cap</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(
		&str, 
		"<span font=\"18\"><tt>%'li</tt></span>", 
		(long)altmarketcap
	);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 2, 1, 1);
	str_dtor(&str);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Bitcoin Dominance</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(&str, "<span font=\"18\"><tt>%g%%</tt></span>", bitcapdom);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 3, 1, 1);
	str_dtor(&str);

	gtk_grid_attach(GTK_GRID(grid), gtk_label_new(NULL), 0, 4, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new(NULL), 1, 4, 1, 1);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Total Volume</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(
		&str, 
		"<span font=\"18\"><tt>%'li</tt></span>", 
		(long)totvolume
	);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 5, 1, 1);
	str_dtor(&str);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Bitcoin Volume</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 6, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(
		&str, 
		"<span font=\"18\"><tt>%'li</tt></span>", 
		(long)bitvolume
	);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 6, 1, 1);
	str_dtor(&str);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Altcoin Volume</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 7, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(
		&str, 
		"<span font=\"18\"><tt>%'li</tt></span>", 
		(long)altvolume
	);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 7, 1, 1);
	str_dtor(&str);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Bitcoin Dominance</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 8, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(&str, "<span font=\"18\"><tt>%g%%</tt></span>", bitvolumedom);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 8, 1, 1);
	str_dtor(&str);

	gtk_grid_attach(GTK_GRID(grid), gtk_label_new(NULL), 0, 9, 1, 1);
	gtk_grid_attach(GTK_GRID(grid), gtk_label_new(NULL), 1, 9, 1, 1);

	label = gtk_label_new(NULL);
	gtk_label_set_markup(
		GTK_LABEL(label), 
		"<span font=\"18\" alpha=\"50%\">Bitcoin Price</span>"
	);
	gtk_label_set_xalign(GTK_LABEL(label), 1.0);
	gtk_grid_attach(GTK_GRID(grid), label, 0, 10, 1, 1);

	label = gtk_label_new(NULL);
	str_ctorfmt(&str, "<span font=\"18\"><tt>%g</tt></span>", bitprice);
	gtk_label_set_markup(GTK_LABEL(label), str.data);
	gtk_label_set_xalign(GTK_LABEL(label), 0.0);
	gtk_widget_set_hexpand(label, 1);
	gtk_grid_attach(GTK_GRID(grid), label, 1, 10, 1, 1);
	str_dtor(&str);

	str_dtor(&httpresult);
	httpclient_delete(http);
	return grid;
}

