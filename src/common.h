#ifndef _COMMON_H
#define _COMMON_H

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

int writeMessage(int fd, void *data, int len);
int readMessage(int fd, void **buf);

#endif
