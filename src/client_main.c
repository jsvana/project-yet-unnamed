#include "common.h"
#include "logging.h"
#include "utils/mqueue.h"

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
mqueue *events;
mqueue *dispEvents;

typedef struct thread_args thread_args;
struct thread_args {
	int sock;
	mqueue *events;
	mqueue *dispEvents;
};

typedef struct msg_command msg_command;
struct msg_command {
	int source;
	char *command;
};

/**
 * Closes in-use resources and exits. Used to catch ^C
 * @param sig Signal passed (unused)
 */
static void cleanup(int sig);

/**
 * Connects to BBS server
 * @param host Host to connect to
 * @param port Port on host
 */
static int joinServer(char *host, int port);

/**
 * Logs message to display window
 * @param msg Message to log
 * @param source Source of message (CLIENT or SERVER)
 */
static void logMessage(const char *msg, int source);

/**
 * Sends message over socket
 * @param sock Socket over which message is sent
 * @param msg Message to send
 */
static void sendMessage(int sock, const char *msg);

/**
 * Receives a message sent over socket, allocates space, and writes to msg
 * @param sock Socket over which message is received
 * @param msg Received message (allocated)
 * @return Number of bytes read
 */
static int receiveMessage(int sock, char **msg);

/**
 * Helper function to run entered command
 * @param command Command to run
 */
static void handleCommand(char *command);

static void *networkThread(void *args);

static void *eventThread(void *args);

int main(int argc, char **argv) {
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

	nodelay(inputW, TRUE);
	scrollok(dispW, TRUE);

	sock = joinServer(argv[1], atoi(argv[2]));

	signal(SIGINT, cleanup);
	signal(SIGSEGV, cleanup);

	mvwaddch(inputW, 0, 0, ACS_ULCORNER);
	for (int i = 1; i < w; i++) {
		mvwaddch(inputW, 0, i, ACS_HLINE);
	}
	mvwaddch(inputW, 1, 0, ACS_VLINE);
	wrefresh(inputW);
	int pos = 0;
	char command[256];
	memset(command, 0, 256);

	events = mqueue_create();
	dispEvents = mqueue_create();

	// Create event thread
	pthread_t networkThreadID;
	thread_args *args = malloc(sizeof(thread_args));
	if (args == NULL) {
		fprintf(stderr, "Error allocating memory\n");
		exit(1);
	}
	args->sock = sock;
	args->events = events;
	args->dispEvents = dispEvents;
	if (pthread_create(&networkThreadID, NULL, networkThread, (void *)args)) {
		fprintf(stderr, "Error creating thread\n");
		exit(1);
	}

	pthread_t eventThreadID;
	if (pthread_create(&eventThreadID, NULL, eventThread, (void *)args)) {
		fprintf(stderr, "Error creating thread\n");
		exit(1);
	}
	free(args);

	sendMessage(sock, "SYN");

	while (1) {
		//char c = mvwgetch(inputW, 1, pos + 1);
		char c = wgetch(inputW);

		if (c == ERR) { // Run through display events
			while (!mqueue_is_empty(dispEvents)) {
				char *e = mqueue_dequeue(dispEvents);
				logMessage(e, CLIENT);
				free(e);
			}
		} else {
			// Check enter
			if (c == 13) {
				for (int i = 1; i <= pos; i++) {
					mvwaddch(inputW, 1, i, ' ');
				}

				command[pos] = 0;
				pos = 0;
				wmove(inputW, 1, 1);

				if (strlen(command) > 0) {
					msg_command *mc = malloc(sizeof(msg_command));
					if (mc == NULL) {
						fprintf(stderr, "Error allocating memory\n");
						exit(1);
					}
					mqueue_enqueue(events, (void *)mc, sizeof(msg_command));
				}
			} else if (c == 127) { // Check backspace
				if (pos > 0) {
					--pos;
					mvwaddch(inputW, 1, pos + 2, ' ');
					command[pos] = 0;
				}
			} else {
				mvwaddch(inputW, 1, pos + 1, c);
				command[pos] = c;
				++pos;
			}
		}
	}

	cleanup(0);

	return 0;
}

static void cleanup(int sig) {
	close(sock);
	mqueue_free(events);
	mqueue_free(dispEvents);

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

	//logMessage(*msg, SERVER);
	mqueue_enqueue(dispEvents, *msg, ret * sizeof(char));

	return ret;
}

static void handleCommand(char *command) {
	commandinfo *cinfo = parseCommand(command, MSG_OUTGOING);

	switch (cinfo->command) {
		case C_QUIT:
			cleanup(0);
			break;

		case C_SYNACK:
			sendMessage(sock, "ACK");
			break;

		case C_GET:
			if (cinfo->param == P_POSTS) {
				sendMessage(sock, command);
			} else if (cinfo->param == P_POST) {
				sendMessage(sock, command);
			}
			break;

		default:
			sendMessage(sock, command);
			break;
	}

	freeCommandInfo(cinfo);
}

static void *networkThread(void *args) {
	thread_args *a = (thread_args *)args;
	char *msg;
	mqueue_enqueue(a->dispEvents, "Network thread started", 23 * sizeof(char));

	while (1) {
		receiveMessage(a->sock, &msg);
		msg_command *mc = malloc(sizeof(msg_command));
		if (mc == NULL) {
			fprintf(stderr, "Error allocating memory\n");
			exit(1);
		}
		mc->source = MSG_OUTGOING;
		mc->command = msg;
		mqueue_enqueue(a->events, (void *)mc, sizeof(msg_command));
		free(mc);
		free(msg);
	}

	return NULL;
}

static void *eventThread(void *args) {
	thread_args *a = (thread_args *)args;
	mqueue_enqueue(a->dispEvents, "Event thread started", 21 * sizeof(char));

	while (1) {
		msg_command *command = mqueue_dequeue(a->events);
		if (command->source == MSG_INCOMING) {
			mqueue_enqueue(a->dispEvents, command->command, strlen(command->command));
		} else {
			handleCommand(command->command);
		}
		free(command);
	}

	return NULL;
}
