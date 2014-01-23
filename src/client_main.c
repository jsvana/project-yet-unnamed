#include "common.h"
#include "logging.h"

#include <arpa/inet.h>
#include <curses.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER 0
#define CLIENT 1

// Globals needed for event handler
int sock;
WINDOW *dispW;
WINDOW *inputW;

static void cleanup(int sig);

static int joinServer(char *host, int port);

static void logMessage(const char *msg, int source);
static void sendMessage(int sock, const char *msg);
static int receiveMessage(int sock, char **msg);

static void handleCommand(char *command);

int main(int argc, char **argv) {
	char *buff;
	char *s = "SYN";
	char *a = "ACK";
	commandinfo *cinfo;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s: <ip> <port>\n", argv[0]);
		exit(1);
	}

	initscr();
	nonl();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	if (has_colors()) {
		start_color();

		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_GREEN, COLOR_BLACK);
		init_pair(3, COLOR_YELLOW, COLOR_BLACK);
		init_pair(4, COLOR_BLUE, COLOR_BLACK);
		init_pair(5, COLOR_CYAN, COLOR_BLACK);
		init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(7, COLOR_WHITE, COLOR_BLACK);
	}

	int h, w;
	getmaxyx(stdscr, h, w);

	dispW = newwin(h - 2, w, 0, 0);
	inputW = newwin(2, w, h - 2, 0);

	scrollok(dispW, TRUE);

	sock = joinServer(argv[1], atoi(argv[2]));

	signal(SIGINT, cleanup);

	sendMessage(sock, s);

	if (!receiveMessage(sock, &buff)) {
		LOG("Server disconnected\n");
		cleanup(0);
	}
	cinfo = parseCommand(buff);
	free(buff);
	if (cinfo->command != C_SYNACK) {
		fprintf(stderr, "Unkown server protocol\n");
		exit(1);
	}

	// Protocol completed, now connected
	sendMessage(sock, a);

	mvwaddch(inputW, 0, 0, ACS_ULCORNER);
	for (int i = 1; i < w; i++) {
		mvwaddch(inputW, 0, i, ACS_HLINE);
	}
	mvwaddch(inputW, 1, 0, ACS_VLINE);
	wrefresh(inputW);
	int pos = 0;
	char command[256];
	memset(command, 0, 256);

	while (1) {
		char c = mvwgetch(inputW, 1, pos + 1);

		// Check enter
		if (c == 13) {
			for (int i = 1; i <= pos; i++) {
				mvwaddch(inputW, 1, i, ' ');
			}

			command[pos] = 0;
			pos = 0;
			wmove(inputW, 1, 1);

			if (strlen(command) > 0) {
				handleCommand(command);
			}
		} else if (c == 127) {
			if (pos > 0) {
				--pos;
				mvwaddch(inputW, 1, pos + 1, ' ');
				command[pos] = 0;
			}
		} else {
			mvwaddch(inputW, 1, pos + 1, c);
			command[pos] = c;
			++pos;
		}
	}

	cleanup(0);

	return 0;
}

static void cleanup(int sig) {
	close(sock);

	endwin();

	exit(0);
}

static int joinServer(char *host, int port) {
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

static void logMessage(const char *msg, int source) {
	char origin = ' ';
	if (source == SERVER) {
		wattron(dispW, COLOR_PAIR(2));
		origin = '<';
	} else if (source == CLIENT) {
		wattron(dispW, COLOR_PAIR(4));
		origin = '>';
	}

	wprintw(dispW, "\n%c %s", origin, msg);

	wrefresh(dispW);
}

static void sendMessage(int sock, const char *msg) {
	writeMessage(sock, (void *)msg, strlen(msg));

	logMessage(msg, CLIENT);
}

static int receiveMessage(int sock, char **msg) {
	int ret = readMessage(sock, (void *)msg);

	logMessage(*msg, SERVER);

	return ret;
}

static void handleCommand(char *command) {
	char *buff;
	commandinfo *cinfo = parseCommand(command);

	switch (cinfo->command) {
		case C_QUIT:
			cleanup(0);
			break;

		case C_GET:
			if (cinfo->param == P_POSTS) {
				sendMessage(sock, command);
				receiveMessage(sock, &buff);
				free(buff);
			} else {
				logMessage(command, CLIENT);
			}
			break;

		default:
			sendMessage(sock, command);
			receiveMessage(sock, &buff);
			free(buff);
			break;
	}

	freeCommandInfo(cinfo);
}
