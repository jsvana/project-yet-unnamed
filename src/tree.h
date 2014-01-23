#pragma once

#include <stdarg.h>

typedef struct config_node config_node;

struct config_node {
	config_node *next;
	config_node *children;
	char *key;
	int type;
	union {
		double d;
		char *s;
		int b;
	} data;
};

config_node *config_find_node(config_node **list, const char *node);
config_node *config_find_for_set(config_node **list, const char *node);

// Helper func, but anyone can use
char **node_name_to_array(const char *node);

//void config_print(config_node *head, int depth);
void config_tag_node(config_node **head, const char *node, int t, ...);
void config_free(config_node **node);
void config_load(config_node **node, const char *file);
