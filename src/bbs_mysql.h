#pragma once

#include <mysql.h>

/**
 * Format string SQL query convenience function (calls bbs_query)
 * @param conn MySQL connection
 * @param fmt Format string
 * @param ... Format string arguments
 * @return Stored MySQL result of the query
 */
MYSQL_RES *bbs_queryf(MYSQL *conn, const char *fmt, ...);

/**
 * MySQL query
 * @param conn MySQL connection
 * @param sql SQL to run
 * @return Stored MySQL result of the query
 */
MYSQL_RES *bbs_query(MYSQL *conn, const char *sql);

/**
 * Escapes MySQL query parameter
 * @param conn MySQL connection
 * @param str String to escape
 * @return Escaped string
 */
char *bbs_escape(MYSQL *conn, const char *str);
