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

commandinfo *parseCommand(char *command) {
	commandinfo *cinfo = malloc(sizeof(commandinfo));
	if (cinfo == NULL) {
		ERR("Error allocating memory\n");
	}
	int len = strlen(command);
	int mark = 0;
	while (command[mark] != ' ' && mark < len) {
		++mark;
	}
	command[mark] = 0;

	if (mark == 0) {
		cinfo->command = NONE;
	} else if (strcmp(command, "SYN") == 0) {
		cinfo->command = SYN;
	} else if (strcmp(command, "SYN/ACK") == 0) {
		cinfo->command = SYNACK;
	} else if (strcmp(command, "ACK") == 0) {
		cinfo->command = ACK;
	} else {
		cinfo->command = UNKNOWN;
	}

	return cinfo;
}
