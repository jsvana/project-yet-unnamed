#ifndef _COMMON_H
#define _COMMON_H

// Best make sure these exist
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// Commands
#define C_UNKNOWN -1
#define C_NONE 0
#define C_SYN 1
#define C_SYNACK 2
#define C_ACK 3
#define C_QUIT 4

typedef struct commandinfo commandinfo;
struct commandinfo {
	int command;
};

int writeMessage(int fd, void *data, int len);
int readMessage(int fd, void **buf);

commandinfo *parseCommand(char *command);

#endif
