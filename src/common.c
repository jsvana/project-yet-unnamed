#include "common.h"

#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
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

static void readn(int fd, void *buf, int len) {
	int bytes = 0;
	int size;

	while (bytes < len) {
		size = read(fd, buf + bytes, len - bytes);
		if (size < 0) {
			perror("Error reading data");
			exit(1);
		}
		bytes += size;
	}
}

int readMessage(int fd, void **buf) {
	int len;
	readn(fd, &len, sizeof(int));

	*buf = malloc(sizeof(char) * len);
	readn(fd, *buf, len);

	return len;
}
