#include "tree.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static void array_free(char **arr) {
	for (int i = 0; arr[i]; i++) {
		free(arr[i]);
	}
	free(arr);
}

config_node *config_find_node_bootstrap(config_node **list, char **nodes, int create) {
	config_node *cur = *list;
	while (cur) {
		if (strcmp(cur->key, nodes[0]) == 0) {
			// Node match
			if (!nodes[1]) {
				// Last node name
				return cur;
			} else {
				return config_find_node_bootstrap(&cur->children, &nodes[1], create);
			}
		}
		cur = cur->next;
	}

	if (create) {
		// Node not found, so prepend to list
		config_node *ret = calloc(1, sizeof(config_node));

		ret->next = *list;
		ret->key = strdup(nodes[0]);
		ret->type = LUA_TNONE;
		ret->children = NULL;

		*list = ret;

		if (nodes[1]) {
			return config_find_node_bootstrap(&ret->children, &nodes[1], create);
		} else {
			return ret;
		}
	}

	return NULL;
}

static config_node *config_find_for_set(config_node **list, const char *node) {
	char **nodes = node_name_to_array(node);
	config_node *ret = config_find_node_bootstrap(list, nodes, 1);
	array_free(nodes);
	return ret;
}

config_node *config_find(config_node **list, const char *node) {
	char **nodes = node_name_to_array(node);
	config_node *ret = config_find_node_bootstrap(list, nodes, 0);
	array_free(nodes);
	return ret;
}

char **node_name_to_array(const char *node) {
	int count = 0;
	char **ret = calloc(count + 1, sizeof(char *));
	if (!ret) {
		return NULL;
	}

	int last_loc = 0;
	for (int i = 0; i < strlen(node); i++) {
		if (node[i] == '.') {
			count += 1;
			char **temp = realloc(ret, (count + 1) * sizeof(char *));
			if (!temp) {
				free(ret);
				return NULL;
			}
			ret = temp;

			// NOTE: if strdup errors, this will have trouble
			ret[count-1] = strndup(&node[last_loc], i - last_loc);
			last_loc = i + 1;
		}
	}

	if (last_loc <= strlen(node)) {
		count += 1;
		char **temp = realloc(ret, (count + 1) * sizeof(char *));
		if (!temp) {
			free(ret);
			return NULL;
		}
		ret = temp;

		ret[count-1] = strdup(&node[last_loc]);
	}

	ret[count] = NULL;

	return ret;
}

/*
   void array_print(char **arr) {
   for (int i = 0; arr[i]; i++) {
   printf("%d (%d). %s\n", i, strlen(arr[i]), arr[i]);
   }
   }
   */

static void config_print(config_node *head, int depth) {
	config_node *cur = head;
	while (cur) {
		printf("%d. %s: %s\n", depth, cur->key, cur->data.s);
		config_print(cur->children, depth+1);
		cur = cur->next;
	}
}

void config_tag_node(config_node **head, const char *node, int t, ...) {
	config_node *tag = config_find_for_set(head, node);
	tag->type = t;
	va_list vl;
	va_start(vl, t);
	switch (t) {
		case LUA_TSTRING:
			tag->data.s = strdup(va_arg(vl, char *));
			break;
		case LUA_TNUMBER:
			tag->data.d = va_arg(vl, double);
			break;
		case LUA_TBOOLEAN:
			tag->data.b = va_arg(vl, int);
			break;
		case LUA_TNONE:
			break;
	}
	va_end(vl);
}

void config_free(config_node **node) {
	config_node *cur = *node;
	config_node *temp = cur;

	while (cur) {
		// Sture the current node in a temp var so we can safely free it
		temp = cur;

		// Women and children first
		config_free(&cur->children);

		// If it's a string, we've strdup'd the value
		if (cur->type == LUA_TSTRING) {
			free(cur->data.s);
		}

		// The key is always a string
		free(cur->key);

		// Move to the next node
		cur = cur->next;

		// Free the current node
		free(temp);
	}
}

static char *str_join(const char *base, const char *join) {
	if (!base) {
		return strdup(join);
	}
	char *ret = calloc(strlen(base) + strlen(join) + 2, sizeof(char));
	strcpy(ret, base);
	strcat(ret, ".");
	return strcat(ret, join);
}

static const char *luaB_types[] = {
	"number",
	"string",
	"boolean",
	"unknown"
};

static const char *luaB_typename(int type) {
	switch (type) {
		case LUA_TNUMBER:
			return luaB_types[0];
			break;
		case LUA_TSTRING:
			return luaB_types[1];
			break;
		case LUA_TBOOLEAN:
			return luaB_types[2];
			break;
		default:
			return luaB_types[3];
			break;
	}
}

static int config_process_stack(lua_State *L, config_node **head, const char *node_name, int depth) {
	int warnings = 0;
	int i = lua_gettop(L);

	lua_pushnil(L);

	while (lua_next(L, i) != 0) {
		// 'key' (at index -2) and 'value' (at index -1)
		int t = lua_type(L, -1);
		if (t == LUA_TTABLE) {
			//printf("%d. %s: %s\n", depth, lua_tostring(L, -2), lua_typename(L, -1));
			config_node *cur = config_find(head, lua_tostring(L, -2));
			if (cur) {
				// Only recurse if we have values to search for there
				char *next_node = str_join(node_name, cur->key);
				warnings += config_process_stack(L, &cur->children, next_node, depth+1);
				free(next_node);
			}
		} else if (t == LUA_TNUMBER || t == LUA_TSTRING || t == LUA_TBOOLEAN) {
			config_node *cur = config_find(head, lua_tostring(L, -2));
			if (!cur) {
				// TODO: Warning, config value not found in known module
			} else if (cur->type != t) {
				// TODO: Warning, type mismatch
				char *node = str_join(node_name, cur->key);
				warnings += 1;
				printf("Type mismatch at value '%s' (expected %s got %s)\n", node, luaB_typename(cur->type), luaB_typename(t));
			} else {
				switch (t) {
					case LUA_TNUMBER:
						cur->data.d = lua_tonumber(L, -1);
						break;
					case LUA_TSTRING:
						if (cur->data.s) {
							free(cur->data.s);
						}
						cur->data.s = strdup(lua_tostring(L, -1));
						break;
					case LUA_TBOOLEAN:
						cur->data.b = lua_toboolean(L, -1);
						break;
				}
			}
			//} else {
			//printf("%d. %s: %s\n", depth, lua_tostring(L, -2), lua_typename(L, -1));
	}

	lua_pop(L, 1);
	}

	return warnings;
}

// TODO: Lua error handling
void config_load(config_node **node, const char *file) {
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);

	if (luaL_dofile(L, file)) {
		printf("%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		return;
	}

	lua_pushglobaltable(L);
	int warnings = config_process_stack(L, node, NULL, 0);
	if (warnings == 1) {
		printf("%d warning occured while loading the config file\n", warnings);
	} else if (warnings) {
		printf("%d warnings occured while loading the config file\n", warnings);
	}

	lua_close(L);
}

/*
void print_node(config_node **config, const char *key) {
	config_node *node = config_find_for_set(config, key);
	if (node) {
		printf("%s (%s): ", key, luaB_typename(node->type));
		switch (node->type) {
			case LUA_TSTRING:
				printf("%s\n", node->data.s);
				break;
			case LUA_TBOOLEAN:
				printf("%s\n", (node->data.b) ? "true" : "false");
				break;
			case LUA_TNUMBER:
				printf("%f\n", node->data.d);
				break;
			default:
				break;
		}
	}
}
*/

/*
this = {
	is = {
		sparta = "world"
	}
}
*/

/*
int main(int argc, char *argv[]) {
	// Config stuff
	config_node *config = NULL;
	config_tag_node(&config, "this.is.sparta", LUA_TSTRING, "world");
	config_tag_node(&config, "this.is.madness", LUA_TSTRING, "hello");
	config_tag_node(&config, "that.is.sparta", LUA_TSTRING, "world");

	config_load(&config, "test.lua");

	// Test-ish
	//config_print(config, 0);

	print_node(&config, "this.is.sparta");

	config_free(&config);
}
*/
