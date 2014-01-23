#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/**
 * MACRO
 * Logs a value to stdout
 * @param fmt String to log
 * @param ... Format arguments
 */
#define LOG(fmt, ...) \
	{ \
		struct timeval t; \
		gettimeofday(&t, NULL); \
		fprintf(stdout, "[LOG %d.%d %s:%d] ", (int)t.tv_sec, (int)t.tv_usec, \
			__FILE__, __LINE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); \
	}

/**
 * MACRO
 * Logs an error to stderr
 * @param fmt String to log
 * @param ... Format arguments
 */
#define ERR(fmt, ...) \
	{ \
		struct timeval t; \
		gettimeofday(&t, NULL); \
		fprintf(stdout, "[ERROR %d.%d %s:%d] ", (int)t.tv_sec, (int)t.tv_usec, \
			__FILE__, __LINE__); \
		fprintf(stdout, fmt, ##__VA_ARGS__); \
	}
