#ifndef __LOGGER__
#define __LOGGER__

#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <linux/limits.h>
#include <unistd.h>

#define LOGGER_LEVEL_INFO		1
#define LOGGER_LEVEL_WARNING	2
#define LOGGER_LEVEL_DEBUG		3
#define LOGGER_LEVEL_VERBOSE	4
#define LOGGER_LEVEL_ERROR		5

#define LOGGER_NAME_MAX			16

typedef struct
{
	int				next;
	bool			in_use;
	
} log_buffer_t;

typedef struct
{
	int					full_buffer_first;
	int					full_buffer_last;
	int					free_buffer_first;
	int					free_buffer_last;
	
	size_t				buffer_size;	
	int					max_buffers;
	int					n_buffers;
	int					log_level;
	log_buffer_t		buffers[0];
	
} logger_buffers_t;


typedef struct
{
	sem_t*	sem;
	sem_t*	available_sem;
	int*	first;
	int*	last;
	
} buffer_queue_t;

typedef struct
{
	int					buffer;
	int					buffer_shm;
	char*				buffer_data;
	
	int					buffers_shm;
	logger_buffers_t*	buffers;
	
	buffer_queue_t		free_buffers;
	buffer_queue_t		full_buffers;
	
	bool				initialized;
	
	char				name[LOGGER_NAME_MAX];
	pthread_mutex_t		mutex;
	int					curr_position;
	
} logger_t;

void map_buffer(logger_t* logger);

void unmap_buffer(logger_t* logger);

void queue_log_buffer(buffer_queue_t* queue, log_buffer_t* buffers, int buffer);

int dequeue_log_buffer(buffer_queue_t* queue, log_buffer_t* buffers);

void init_buffer_queues(logger_t* logger, bool server);

void init_buffers(logger_t* logger, int n_buffers);

void init_logger(logger_t* logger, bool server, bool force, int log_level, int n_buffers, size_t buffer_size);

void remove_files();

#endif
