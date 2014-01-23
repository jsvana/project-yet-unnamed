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
#define C_QUIT 1
#define C_SYN 2
#define C_SYNACK 3
#define C_ACK 4
#define C_POSTS 5

// Params
#define P_UNKNOWN -1
#define P_NONE 0
#define P_POSTS 1

typedef struct commandinfo commandinfo;
struct commandinfo {
	int command;
	int param;
};

int writeMessage(int fd, void *data, int len);
int readMessage(int fd, void **buf);

commandinfo *parseCommand(char *command);

char *protocolEscape(const char *str);
char *protocolUnescape(const char *str);

#endif
