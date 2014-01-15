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

#define MAX_CONNECTIONS 10

void clientFunc(int sock, struct sockaddr_in *sockInfo);

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(1);
	}

	int port = atoi(argv[1]);

	int servSock = socket(AF_INET, SOCK_STREAM, 0);
	if (servSock < 0) {
		perror("Error creating socket");
		exit(1);
	}

	int reuse = TRUE;
	int ret;

	ret = setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));
	if (ret < 0) {
		perror("Error setting socket option");
		exit(1);
	}

	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));

	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(port);

	ret = bind(servSock, (struct sockaddr *)&serv, sizeof(struct sockaddr_in));
	if (ret < 0) {
		perror("Error binding to socket");
		exit(1);
	}

	ret = listen(servSock, MAX_CONNECTIONS);
	if (ret < 0) {
		perror("Error initiating listen");
		exit(1);
	}

	LOG("Server listening on port %d\n", port);

	int running = TRUE;

	while (running) {
		struct sockaddr_in clientInfo;
		socklen_t structSize = sizeof(struct sockaddr_in);
		int sock = accept(servSock, (struct sockaddr *)&clientInfo, &structSize);

		int cpid = fork();

		if (cpid < 0) {
			ERR("Error creating child\n");
		} else if (cpid == 0) {
			clientFunc(sock, &clientInfo);
		}
	}

	return 0;
}

void clientFunc(int sock, struct sockaddr_in *sockInfo) {
	char *sa = "SYN/ACK";
	char *buff;
	commandinfo *cinfo;

	unsigned int ip = ntohl(sockInfo->sin_addr.s_addr);
	LOG("Client connected at IP %d.%d.%d.%d\n", ip >> 24 & 255,
		ip >> 16 & 255, ip >> 8 & 255, ip & 255);

	readMessage(sock, (void *)&buff);
	cinfo = parseCommand(buff);
	free(buff);
	if (cinfo->command != SYN) {
		fprintf(stderr, "Unknown client protocol\n");
		close(sock);
		exit(1);
	}

	writeMessage(sock, (void *)sa, strlen(sa));

	readMessage(sock, (void *)&buff);
	cinfo = parseCommand(buff);
	free(buff);
	if (cinfo->command != ACK) {
		fprintf(stderr, "Unknown client protocol\n");
		close(sock);
		exit(1);
	}

	int running = TRUE;
	int s;

	while (running) {
		s = readMessage(sock, (void *)&buff);
		if (s == 0) {
			LOG("Client %d.%d.%d.%d disconnected\n", ip >> 24 & 255, ip >> 16 & 255,
				ip >> 8 & 255, ip & 255);
			running = FALSE;
			break;
		}

		writeMessage(sock, (void *)buff, strlen(buff));
		LOG("Received \"%s\"\n", buff);
		cinfo = parseCommand(buff);
		free(buff);
	}

	exit(0);
}
