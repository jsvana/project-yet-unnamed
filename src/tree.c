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

config_node *config_find_node(config_node **list, const char *node) {
	char **nodes = node_name_to_array(node);
	config_node *ret = config_find_node_bootstrap(list, nodes, 1);
	array_free(nodes);
	return ret;
}

config_node *config_find_for_set(config_node **list, const char *node) {
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

/*
void config_print(config_node *head, int depth) {
	config_node *cur = head;
	while (cur) {
		printf("%d. %s: %s\n", depth, cur->key, cur->data.s);
		config_print(cur->children, depth+1);
		cur = cur->next;
	}
}
*/

void config_tag_node(config_node **head, const char *node, int t, ...) {
	config_node *tag = config_find_node(head, node);
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

static void config_process_stack(lua_State *L, config_node **head) {
	int i = lua_gettop(L);

	lua_pushnil(L);
	
	while (lua_next(L, i) != 0) {
		// 'key' (at index -2) and 'value' (at index -1)
		int t = lua_type(L, -1);
		if (t == LUA_TTABLE) {
			config_node *cur = config_find_for_set(head, lua_tostring(L, -2));
			if (cur) {
				config_process_stack(L, &cur->children);
			} else {
				config_process_stack(L, NULL);
			}
		} else if (t == LUA_TNUMBER || t == LUA_TSTRING || t == LUA_TBOOLEAN) {
			config_node *cur = config_find_for_set(head, lua_tostring(L, -2));
			if (!cur) {
				// TODO: Warning, config value not found
			} else if (cur->type != t) {
				// TODO: Warning, type mismatch
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
		}

		lua_pop(L, 1);
	}
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

	lua_getglobal(L, "conf");
	config_process_stack(L, node);

	lua_close(L);
}

/*

conf = {
	this = {
		is = {
			madness = "world",
			sparta = "hello",
		}
	}
}

int main(int argc, char *argv[]) {
	// Config stuff
	config_node *config = NULL;
	config_tag_node(&config, "this.is.sparta", LUA_TSTRING, "world");
	config_tag_node(&config, "this.is.madness", LUA_TSTRING, "hello");
	config_tag_node(&config, "that.is.sparta", LUA_TSTRING, "world");

	config_load(&config, "test.lua");

	// Test-ish
	config_print(config, 0);

	config_free(&config);
}
*/
