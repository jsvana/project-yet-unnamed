#include "common.h"
#include "logging.h"

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int joinServer(char *host, int port) {
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("Error opening socket");
		exit(1);
	}

	struct sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = inet_addr(host);

	if (connect(sock, (struct sockaddr *)&serv, sizeof(struct sockaddr_in)) < 0) {
		perror("Error connecting to socket");
		close(sock);
		exit(1);
	}

	return sock;
}

int main(int argc, char **argv) {
	if (argc < 3) {
		fprintf(stderr, "Usage: %s: <ip> <port>\n", argv[0]);
		exit(1);
	}

	int sock = joinServer(argv[1], atoi(argv[2]));

	char *buff;

	readMessage(sock, (void *)&buff);

	printf("%s", buff);

	free(buff);

	char *m = "Test message\n";

	writeMessage(sock, (void *)m, strlen(m));

	readMessage(sock, (void *)&buff);

	printf("%s", buff);

	free(buff);

	return 0;
}
