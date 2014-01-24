#include "mqueue.h"

#include "../logging.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

static mqueue_node *mqueue_node_create() {
	mqueue_node *mn = malloc(sizeof(mqueue_node));
	if (mn == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}

	mn->data = NULL;
	mn->size = 0;
	mn->next = NULL;
	mn->prev = NULL;

	return mn;
}

static void mqueue_node_free(mqueue_node *mn) {
	if (mn == NULL) {
		return;
	}

	if (mn->data != NULL) {
		free(mn->data);
	}

	free(mn);
}

mqueue *mqueue_create() {
	mqueue *m = malloc(sizeof(mqueue));
	if (m == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}

	if (pthread_mutex_init(&m->lock, NULL)) {
		ERR("Error creating mutex\n");
		exit(1);
	}
	if (pthread_cond_init(&m->empty, NULL)) {
		ERR("Error creating condition variable\n");
		exit(1);
	}
	m->size = 0;
	m->head = mqueue_node_create();
	m->tail = mqueue_node_create();
	m->head->next = m->tail;
	m->tail->prev = m->head;

	return m;
}

void mqueue_enqueue(mqueue *m, void *data, size_t size) {
	if (m == NULL) {
		return;
	}

	mqueue_node *mn = mqueue_node_create();

	mn->data = malloc(size);
	if (mn->data == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}
	memcpy(mn->data, data, size);
	mn->size = size;

	pthread_mutex_lock(&m->lock);

	mqueue_node *t = m->tail;
	mqueue_node *n = t->prev;
	n->next = mn;
	mn->prev = n;
	mn->next = t;
	t->prev = mn;
	++m->size;
	if (m->size == 1) {
		pthread_cond_broadcast(&m->empty);
	}

	pthread_mutex_unlock(&m->lock);
}

void *mqueue_peek(mqueue *m) {
	if (m == NULL || m->size == 0) {
		return NULL;
	}

	pthread_mutex_lock(&m->lock);
	return m->head->next->data;
	pthread_mutex_unlock(&m->lock);
}

void *mqueue_dequeue(mqueue *m) {
	if (m == NULL) {
		return NULL;
	}

	pthread_mutex_lock(&m->lock);

	while (m->size == 0) {
		pthread_cond_wait(&m->empty, &m->lock);
	}

	mqueue_node *n = m->head->next;
	void *data = malloc(n->size);
	if (data == NULL) {
		ERR("Error allocating memory\n");
		exit(1);
	}
	memcpy(data, n->data, n->size);

	mqueue_node *nn = n->next;
	m->head->next = nn;
	nn->prev = m->head;
	--m->size;

	pthread_mutex_unlock(&m->lock);

	mqueue_node_free(n);

	return data;
}

int mqueue_size(mqueue *m) {
	if (m == NULL) {
		return 0;
	}

	int size;
	pthread_mutex_lock(&m->lock);
	size = m->size;
	pthread_mutex_unlock(&m->lock);

	return size;
}

int mqueue_is_empty(mqueue *m) {
	return mqueue_size(m) == 0;
}

void mqueue_dequeue_noret(mqueue *m) {
	if (m == NULL) {
		return;
	}

	pthread_mutex_lock(&m->lock);

	if (m->size == 0) {
		pthread_cond_wait(&m->empty, &m->lock);
	}

	mqueue_node *n = m->head->next;

	mqueue_node *nn = n->next;
	m->head->next = nn;
	nn->prev = m->head;
	--m->size;

	mqueue_node_free(n);

	pthread_mutex_unlock(&m->lock);
}

void mqueue_free(mqueue *m) {
	if (m == NULL) {
		return;
	}

	while (!mqueue_is_empty(m)) {
		mqueue_dequeue_noret(m);
	}

	free(m);
}
