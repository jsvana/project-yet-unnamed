#pragma once

#include <sys/types.h>

// Best make sure these exist
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// Message sources
#define MSG_INCOMING 0
#define MSG_OUTGOING 1

// Commands
#define C_UNKNOWN -1
#define C_NONE 0
#define C_QUIT 1
#define C_SYN 2
#define C_SYNACK 3
#define C_ACK 4
#define C_POSTS 5
#define C_GET 6
#define C_ERROR 7

// Params
#define P_UNKNOWN -1
#define P_NONE 0
#define P_POSTS 1
#define P_POST 2

// Struct representing user in database
typedef struct user user;
struct user {
	long long id;
	char *username;
	char *password;
};

// Struct representing post in database
typedef struct post post;
struct post {
	long long id;
	long long authorID;
	char *title;
	char *postDate;
	char *content;
};

// Struct to hold parsed command data to and from server
typedef struct commandinfo commandinfo;
struct commandinfo {
	int command;
	int param;
	int argCount;
	char **args;
};

// This doesn't exist on Linux...why?
#ifdef __linux__
/**
 * Wrapper around realloc that will free ptr if realloc fails
 *
 * From `man reallocf`:
 * The `reallocf`() function is identical to the `realloc`() function, except
 * that it will free the passed pointer when the requested memory cannot be
 * allocated. This is a FreeBSD specific API designed to ease the problems with
 * traditional coding styles for realloc causing memory leaks in libraries.
 *
 * @param ptr Pointer to be reallocated
 * @param size New size of block
 * @return Newly allocated block, or NULL if failure
 */
void *reallocf(void *ptr, size_t size);
#endif

/**
 * Writes a message to the fd (socket)
 *
 * TODO: Add magic number
 *
 * First writes one int giving the length of data then writes the data
 *
 * @param fd File descriptor to which data is written
 * @param data Data to write
 * @param len Length of data to write
 * @return Number of bytes written
 */
int writeMessage(int fd, void *data, int len);

/**
 * Reads a message from fd, allocates space in buf, and writes into buf
 *
 * TODO: Add magic number
 *
 * @param fd File descriptor from which data is read
 * @param buf Double pointer to buf (for allocation)
 * @return Number of bytes read
 */
int readMessage(int fd, void **buf);

/**
 * Parses command string into commandinfo struct
 *
 * Escapes and unescapes values according to `source` parameter
 * If source is MSG_INCOMING, values will be unescaped. Otherwise, values will
 * be escaped.
 *
 * @param command Command to be parsed
 * @param source Source of command
 * @return Parsed commandinfo struct
 */
commandinfo *parseCommand(char *command, int incoming);

/**
 * Frees the given commandinfo struct and all data within
 */
void freeCommandInfo(commandinfo *cinfo);

/**
 * Escapes a string to be sent, according to the protocol
 * @param str String to be escaped
 * @return Escaped string
 */
char *protocolEscape(const char *str);

/**
 * Escapes a string of a given length to be sent, according to the protocol
 * @param str String to be escaped
 * @param n Length of string
 * @return Escaped string
 */
char *protocolEscapen(const char *str, int n);

/**
 * Unescapes a string to be received, according to the protocol
 * @param str String to be escaped
 * @param n Length of string
 * @return Unescaped string
 */
char *protocolUnescape(const char *str);

/**
 * Unescapes a string of a given length to be received,
 * according to the protocol
 *
 * @param str String to be escaped
 * @param n Length of string
 * @return Unescaped string
 */
char *protocolUnescapen(const char *str, int n);
