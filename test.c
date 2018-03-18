#include "test.h"

enum ColumnType
{
	COLUMNTYPE_NAME,
	COLUMNTYPE_MARKETCAP,
	COLUMNTYPE_PRICE,
	COLUMNTYPE_CHANGE,
	NUM_COLUMNTYPES
};

void populate_store(GtkListStore* store)
{
	const char* data[][4] = {
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
		{"Bitcoin", "178,123,123,334", "8172.23", "-4.6"},
	};

	GtkTreeIter iter;
	for(size_t i = 0; i < sizeof data / sizeof *data; i++)
	{
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(
			store, 
			&iter, 
			COLUMNTYPE_NAME, data[i][0],
			COLUMNTYPE_MARKETCAP, data[i][1],
			COLUMNTYPE_PRICE, data[i][2],
			COLUMNTYPE_CHANGE, data[i][3],
			-1
		);
	}
}

void add_column(GtkTreeView* tree, const char* title, enum ColumnType type)
{
	GtkCellRenderer* renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes(
		title,
		renderer,
		"text", type,
		NULL
	);

	gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_sort_column_id(column, type);
	gtk_tree_view_column_set_expand(column, 1);
	gtk_tree_view_column_set_resizable(column, 1);
	gtk_tree_view_append_column(tree, column);
}

GtkWidget* allpage_new(void)
{
	GtkListStore* store = gtk_list_store_new(
		NUM_COLUMNTYPES, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING, 
		G_TYPE_STRING
	);

	populate_store(store);
	GtkWidget* tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(tree), 1);

	const char* titles[] = {"Name", "Market Cap", "Price", "Change"};
	for(size_t i = 0; i < NUM_COLUMNTYPES; i++)
	{
		add_column(GTK_TREE_VIEW(tree), titles[i], i);
	}

	GtkWidget* scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolled), tree);
	g_object_unref(G_OBJECT(store));

	return scrolled;
}

