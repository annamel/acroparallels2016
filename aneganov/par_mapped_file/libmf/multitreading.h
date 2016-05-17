#ifndef __MF_MULTITREADING_H
#define __MF_MULTITREADING_H

#include <pthread.h>

struct shared_mutex {
	pthread_mutex_t read;
	pthread_mutex_t write;
	int read_count;
};

static inline void read_shared_lock(struct shared_mutex *m) {
	pthread_mutex_lock(&m->read);
	if(++m->read_count == 1) {
		pthread_mutex_lock(&m->write);
	}
	pthread_mutex_unlock(&m->read);
}

static inline void read_shared_unlock(struct shared_mutex *m) {
	pthread_mutex_lock(&m->read);
	if(--m->read_count == 0) {
		pthread_mutex_unlock(&m->write);
	}
	pthread_mutex_unlock(&m->read);
}

static inline void write_exclusive_lock(struct shared_mutex *m) {
	pthread_mutex_lock(&m->write);
}

static inline void write_exclusive_unlock(struct shared_mutex *m) {
	pthread_mutex_unlock(&m->write);
}

#endif