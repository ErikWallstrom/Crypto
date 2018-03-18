#include "crypto.h"
#include "log.h"
#include "httpclient.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <json-glib/json-glib.h>

enum Column
{
	COLUMN_NAME,
	COLUMN_MARKETCAP,
	COLUMN_PRICE,
	COLUMN_CHANGE,
};

static gint name_sort(
	GtkTreeModel* model, 
	GtkTreeIter* a, 
	GtkTreeIter* b, 
	void* userdata)
{
	(void)userdata;

	gchar* name1;
	gchar* name2;
	gtk_tree_model_get(model, a, COLUMN_NAME, &name1, -1);
	gtk_tree_model_get(model, b, COLUMN_NAME, &name2, -1);

	size_t len1 = strlen(name1);
	size_t len2 = strlen(name2);
	size_t minlen = (len1 > len2) ? len2 : len1;

	gint ret = 0;
	for(size_t i = 0; i < minlen; i++)
	{
		if(tolower(name1[i]) > tolower(name2[i]))
		{
			ret = 1;
			break;
		}
		else if(tolower(name2[i]) > tolower(name1[i]))
		{
			ret = -1;
			break;
		}
	}

	g_free(name1);
	g_free(name2);
	return ret;
}

static gint marketcap_sort(
	GtkTreeModel* model, 
	GtkTreeIter* a, 
	GtkTreeIter* b, 
	void* userdata)
{
	(void)userdata;

	gchar* name1;
	gchar* name2;
	gtk_tree_model_get(model, a, COLUMN_MARKETCAP, &name1, -1);
	gtk_tree_model_get(model, b, COLUMN_MARKETCAP, &name2, -1);

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
	gtk_tree_model_get(model, a, COLUMN_PRICE, &name1, -1);
	gtk_tree_model_get(model, b, COLUMN_PRICE, &name2, -1);

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
	gtk_tree_model_get(model, a, COLUMN_CHANGE, &name1, -1);
	gtk_tree_model_get(model, b, COLUMN_CHANGE, &name2, -1);

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

static void name_cell_data(
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
}

void crypto_init(GtkWidget* window)
{
	log_assert(window, "is NULL");

	GtkListStore* store = gtk_list_store_new(
		4, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING,
		G_TYPE_STRING
	);

	GtkTreeSortable* sortable = GTK_TREE_SORTABLE(store);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMN_NAME, 
		name_sort, 
		NULL, 
		NULL
	);
	gtk_tree_sortable_set_sort_column_id(
		sortable, 
		COLUMN_MARKETCAP, 
		GTK_SORT_DESCENDING
	);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMN_MARKETCAP, 
		marketcap_sort, 
		NULL, 
		NULL
	);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMN_PRICE, 
		price_sort, 
		NULL, 
		NULL
	);
	gtk_tree_sortable_set_sort_func(
		sortable, 
		COLUMN_CHANGE, 
		change_sort, 
		NULL, 
		NULL
	);

	struct HTTPClient* httpclient = httpclient_new("coincap.io/front");
	struct Str str = httpclient_get(httpclient);
	GError* error = NULL;
	JsonNode* root = json_from_string(str.data, &error);
	if(error)
	{
		log_error("%s", error->message);
		g_error_free(error);
	}

	JsonArray* array = json_node_get_array(root);
	guint len = json_array_get_length(array);
	log_info("%u", len);
	for(guint i = 0; i < ((len > 100) ? 100 : len); i++)
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
	httpclient_delete(httpclient);

	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
	g_object_set(renderer, "font", "Monospace 14", NULL);

	GtkWidget* tree = gtk_tree_view_new();
	gtk_tree_view_set_model(GTK_TREE_VIEW(tree), GTK_TREE_MODEL(store));
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(tree), 1);


	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
		"Name",
		renderer,
		"text", COLUMN_NAME,
		NULL
	);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_NAME);
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

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_MARKETCAP);
	gtk_tree_view_column_set_expand(column, 1);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes(
		"Price",
		renderer,
		"text", COLUMN_PRICE,
		NULL
	);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_PRICE);
	gtk_tree_view_column_set_expand(column, 1);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes(
		"%24hr",
		renderer,
		"text", COLUMN_CHANGE,
		NULL
	);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(column, COLUMN_CHANGE);
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
}
