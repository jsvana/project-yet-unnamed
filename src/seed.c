#include "bbs_mysql.h"
#include "common.h"
#include "logging.h"

#include <stdlib.h>
#include <string.h>

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

	user jsvana;
	jsvana.username = bbs_escape(conn, "jsvana");

	int postCount = 2;
	post posts[postCount];
	posts[0].title = bbs_escape(conn, "First Post");
	posts[0].content = bbs_escape(conn, "Hello there!\nThis is my first post.");

	posts[1].title = bbs_escape(conn, "Another Post");
	posts[1].content = bbs_escape(conn, "Here's another post. \"What?\"");

	MYSQL_RES *res = bbs_query(conn, "TRUNCATE TABLE `users`");
	mysql_free_result(res);
	res = bbs_queryf(conn, "INSERT INTO `users` (`username`)"
		" VALUES ('%s')", jsvana.username);
	long long id = mysql_insert_id(conn);
	mysql_free_result(res);

	res = bbs_query(conn, "TRUNCATE TABLE `posts`");
	mysql_free_result(res);

	for (int i = 0; i < postCount; i++) {
		res = bbs_queryf(conn, "INSERT INTO `posts` (`creator_id`, `post_date`,"
			" `content`, `title`) VALUES (%lld, NOW(), '%s', '%s')", id,
			posts[i].content, posts[i].title);
		mysql_free_result(res);
	}

	mysql_close(conn);

	return 0;
}
