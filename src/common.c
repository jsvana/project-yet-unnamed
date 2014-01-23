#include "common.h"

#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

	int mark = 0;
	int len = strlen(command);
	char c = ' ';
	while (mark < strlen(command)) {
	}
}

commandinfo *parseCommand(char *command) {
	commandinfo *cinfo = malloc(sizeof(commandinfo));
	if (cinfo == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}
	int len = strlen(command);
	int mark = 0;
	while (command[mark] != ' ' && mark < len) {
		++mark;
	}
	command[mark] = 0;

	if (mark == 0) {
		cinfo->command = C_NONE;
	} else if (strcmp(command, "QUIT") == 0) {
		cinfo->command = C_QUIT;
	} else if (strcmp(command, "SYN") == 0) {
		cinfo->command = C_SYN;
	} else if (strcmp(command, "SYN/ACK") == 0) {
		cinfo->command = C_SYNACK;
	} else if (strcmp(command, "ACK") == 0) {
		cinfo->command = C_ACK;
	} else if (strcmp(command, "POSTS") == 0) {
		cinfo->command = C_POSTS;
	} else if (strcmp(command, "GET") == 0) {
		cinfo->command = C_POSTS;
		if (strcmp())
	} else {
		cinfo->command = C_UNKNOWN;
	}

	return cinfo;
}

char *protocolEscape(const char *str) {
	char *out = malloc(sizeof(char) * strlen(str) * 2);
	int outIndex = 0;

	for (int i = 0; i < strlen(str); i++) {
		if (str[i] == '"') {
			out[outIndex++] = '\\';
			out[outIndex++] = '"';
		} else {
			out[outIndex++] = str[i];
		}
	}

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
