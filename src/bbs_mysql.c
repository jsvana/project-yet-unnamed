#include "bbs_mysql.h"

#include "logging.h"

#include <mysql.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

MYSQL_RES *bbs_queryf(MYSQL *conn, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char *sql;
	vasprintf(&sql, fmt, ap);
	va_end(ap);
	if (sql == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}

	return bbs_query(conn, sql);
}

MYSQL_RES *bbs_query(MYSQL *conn, const char *sql) {
	if (mysql_query(conn, sql)) {
		ERR("Error querying database: %s\n", mysql_error(conn));
		exit(1);
	}

	return mysql_store_result(conn);
}

char *bbs_escape(MYSQL *conn, const char *str) {
	char *to = malloc(sizeof(char) * (strlen(str) * 2 + 1));
	memset(to, 0, strlen(str) * 2 + 1);
	if (to == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}

	mysql_real_escape_string(conn, to, str, strlen(str));

	return to;
}
