#include "../mqueue.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	mqueue *m = mqueue_create();
	int i = 5;
	mqueue_enqueue(m, (void *)&i, sizeof(int));
	i = 6;
	mqueue_enqueue(m, (void *)&i, sizeof(int));
	i = 7;
	mqueue_enqueue(m, (void *)&i, sizeof(int));

	while (!mqueue_is_empty(m)) {
		int *data = mqueue_dequeue(m);
		printf("%d\n", *data);
		free(data);
	}

	mqueue_free(m);

	return 0;
}
