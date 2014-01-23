#include "logging.h"

#include <mysql.h>
#include <stdio.h>

void cleanup(MYSQL *conn) {
	ERR("Error: %s\n", mysql_error(conn));
	mysql_close(conn);
	exit(1);
}

int main(int argc, char **argv) {
	MYSQL *conn = mysql_init(NULL);
	if (conn == NULL) {
		ERR("Error: %s\n", mysql_error(conn));
		exit(1);
	}

	if (mysql_real_connect(conn, "localhost", "root", "linked", "bbs", 0, NULL, 0)
			== NULL) {
		ERR("Error: %s\n", mysql_error(conn));
		exit(1);
	}

	if (mysql_query(conn, "SELECT * FROM `posts` WHERE `id`=1")) {
		cleanup(conn);
	}

	MYSQL_RES *res = mysql_store_result(conn);
	MYSQL_ROW row = mysql_fetch_row(res);

	printf("Post %s: %s\n", row[0], row[3]);

	mysql_free_result(res);
	mysql_close(conn);

	return 0;
}
