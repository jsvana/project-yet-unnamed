#pragma once

#include <mysql.h>

MYSQL_RES *bbs_queryf(MYSQL *conn, const char *fmt, ...);
MYSQL_RES *bbs_query(MYSQL *conn, const char *sql);
