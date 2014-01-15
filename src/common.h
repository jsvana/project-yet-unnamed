#ifndef _COMMON_H
#define _COMMON_H

// Best make sure these exist
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// Commands
#define UNKNOWN -1
#define NONE 0
#define SYN 1
#define SYNACK 2
#define ACK 3

typedef struct commandinfo commandinfo;
struct commandinfo {
	int command;
};

int writeMessage(int fd, void *data, int len);
int readMessage(int fd, void **buf);

commandinfo *parseCommand(char *command);

#endif
