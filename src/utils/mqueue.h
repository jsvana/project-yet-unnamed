#pragma once

#include <pthread.h>

typedef struct mqueue_node mqueue_node;
struct mqueue_node {
	void *data;
	size_t size;
	mqueue_node *next;
	mqueue_node *prev;
};

typedef struct mqueue mqueue;
struct mqueue {
	pthread_mutex_t lock;
	pthread_cond_t empty;
	int size;
	mqueue_node *head;
	mqueue_node *tail;
};

mqueue *mqueue_create();

void mqueue_enqueue(mqueue *m, void *data, size_t size);
void *mqueue_peek(mqueue *m);
void *mqueue_dequeue(mqueue *m);
void mqueue_dequeue_noret(mqueue *m);
int mqueue_size(mqueue *m);
int mqueue_is_empty(mqueue *m);

void mqueue_free(mqueue *m);
