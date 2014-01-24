#include "../mqueue.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t printLock = PTHREAD_MUTEX_INITIALIZER;

void *producerThread(void *args) {
	mqueue *events = (mqueue *)args;
	for (int i = 0; i < 10; i++) {
		mqueue_enqueue(events, (void *)&i, sizeof(int));
		pthread_mutex_lock(&printLock);
		printf("Queuing %d\n", i);
		pthread_mutex_unlock(&printLock);
	}
	return NULL;
}

void *consumerThread(void *args) {
	mqueue *events = (mqueue *)args;
	for (int i = 0; i < 5; i++) {
		int *j = mqueue_dequeue(events);
		pthread_mutex_lock(&printLock);
		printf("Dequeuing %d\n", *j);
		pthread_mutex_unlock(&printLock);
		free(j);
	}
	return NULL;
}

int main(int argc, char **argv) {
	pthread_t producer;
	pthread_t consumers[2];
	mqueue *events = mqueue_create();

	pthread_create(&producer, NULL, producerThread, (void *)events);
	pthread_create(&consumers[0], NULL, consumerThread, (void *)events);
	pthread_create(&consumers[1], NULL, consumerThread, (void *)events);

	pthread_join(producer, NULL);
	pthread_join(consumers[0], NULL);
	pthread_join(consumers[1], NULL);

	return 0;
}
