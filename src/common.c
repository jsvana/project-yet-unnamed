#include "common.h"

#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#ifdef __linux__

void *reallocf(void *ptr, size_t size) {
	void *p = realloc(ptr, size);
	if (p == NULL) {
		free(ptr);
		return NULL;
	}
	return p;
}

#endif

int writeMessage(int fd, void *data, int len) {
	int ret;

	ret = write(fd, &len, sizeof(int));
	if (ret < 0) {
		return ret;
	}
	ret = write(fd, data, len);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

static int readn(int fd, void *buf, int len) {
	int bytes = 0;
	int size;

	while (bytes < len) {
		size = read(fd, buf + bytes, len - bytes);
		if (size == 0) {
			return 0;
		} else if (size < 0) {
			perror("Error reading data");
			exit(1);
		}
		bytes += size;
	}

	return bytes;
}

int readMessage(int fd, void **buf) {
	int len;
	int size;
	size = readn(fd, &len, sizeof(int));

	if (size == 0) {
		return 0;
	}

	*buf = malloc(sizeof(char) * len + 1);
	memset(*buf, 0, len + 1);
	size = readn(fd, *buf, len);

	if (size == 0) {
		free(*buf);
		return 0;
	}

	return len;
}

static char **parseArguments(char *command) {
	char **args = malloc(sizeof(char *));
	if (args == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}

	int argCtr = 0;
	int mark = 0;
	int len = strlen(command);
	char c = ' ';
	char *p = command;
	while (mark < len) {
		args = reallocf(args, sizeof(char *) * (argCtr + 1));
		if (args == NULL) {
			ERR("Error allocating memory\n");
			exit(1);
		}

		if (p[mark] == '"') {
			++mark;
			c = '"';
		} else {
			c = ' ';
		}

		p += mark;
		len -= mark;
		mark = 0;

		while (p[mark] != c && mark < len) {
			++mark;
		}

		args[argCtr] = malloc(sizeof(char) * (mark + 1));
		if (args[argCtr] == NULL) {
			ERR("Error allocating memory\n");
			exit(1);
		}
		strncpy(args[argCtr], p, mark);
		args[argCtr][mark] = 0;
		if (c == '"') {
			++mark;
		}
		++mark;
		++argCtr;
	}

	args = reallocf(args, sizeof(char *) * (argCtr + 1));
	if (args == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}
	args[argCtr] = NULL;

	return args;
}

static void freeArguments(char **args) {
	if (args == NULL) {
		return;
	}

	int i = 0;
	while (args[i]) {
		free(args[i]);
		++i;
	}
}

commandinfo *parseCommand(char *command) {
	commandinfo *cinfo = malloc(sizeof(commandinfo));
	if (cinfo == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}

	cinfo->args = parseArguments(command);
	char **args = cinfo->args;
	cinfo->argCount = 0;
	while (*args) {
		++cinfo->argCount;
		args++;
	}

	if (!*cinfo->args) {
		cinfo->command = C_NONE;
	} else if (strcmp(cinfo->args[0], "QUIT") == 0) {
		cinfo->command = C_QUIT;
	} else if (strcmp(cinfo->args[0], "SYN") == 0) {
		cinfo->command = C_SYN;
	} else if (strcmp(cinfo->args[0], "SYN/ACK") == 0) {
		cinfo->command = C_SYNACK;
	} else if (strcmp(cinfo->args[0], "ACK") == 0) {
		cinfo->command = C_ACK;
	} else if (strcmp(cinfo->args[0], "POSTS") == 0) {
		cinfo->command = C_POSTS;
	} else if (strcmp(cinfo->args[0], "GET") == 0) {
		cinfo->command = C_GET;
		if (cinfo->args[1]) {
			if (strcmp(cinfo->args[1], "POSTS") == 0) {
				cinfo->param = P_POSTS;
			} else if (strcmp(cinfo->args[1], "POST") == 0) {
				cinfo->param = P_POST;
			} else {
				cinfo->param = P_UNKNOWN;
			}
		} else {
			cinfo->param = P_NONE;
		}
	} else if (strcmp(cinfo->args[0], "ERROR") == 0) {
		cinfo->command = C_ERROR;
	} else {
		cinfo->command = C_UNKNOWN;
	}

	return cinfo;
}

void freeCommandInfo(commandinfo *cinfo) {
	if (cinfo == NULL) {
		return;
	}

	freeArguments(cinfo->args);
	free(cinfo);
}

char *protocolEscape(const char *str) {
	char *out = malloc(sizeof(char) * (strlen(str) * 2 + 1));
	int outIndex = 0;

	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == '"') {
			out[outIndex++] = '\\';
			out[outIndex++] = '"';
		} else {
			out[outIndex++] = str[i];
		}
	}

	out[outIndex] = 0;

	return out;
}

char *protocolUnescape(const char *str) {
	char *out = malloc(sizeof(char) * strlen(str));
	int outIndex = 0;
	int len = strlen(str);

	for (int i = 0; i < len; i++) {
		if (i < len && str[i] == '\\' && str[i + 1] == '"') {
			out[outIndex++] = '"';
			++i;
		} else {
			out[outIndex++] = str[i];
		}
	}

	return out;
}
