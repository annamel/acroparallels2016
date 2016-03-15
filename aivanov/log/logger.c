#include "logger.h"
#include <errno.h>
#include <string.h>

#define LOGGER_SHM_NAME							"/logger_shm"
#define LOGGER_BUFFER_SHM_NAME					"/logger_buffer_shm_%d"
#define LOGGER_FULL_BUFFERS_SEM_NAME			"/logger_full_buffers_sem"
#define LOGGER_FULL_BUFFERS_AVAILABLE_SEM_NAME	"/logger_full_buffers_available_sem"
#define LOGGER_FREE_BUFFERS_SEM_NAME			"/logger_free_buffers_sem"
#define LOGGER_FREE_BUFFERS_AVAILABLE_SEM_NAME	"/logger_free_buffers_available_sem"

#define LOGGER_FILES_MODE 0666
//S_IRUSR | S_IWUSR
#define LOGGER_CREATION_OFLAGS (O_CREAT | O_EXCL)

void map_buffer(logger_t* logger)
{
	char shm_name[PATH_MAX];
	sprintf(shm_name, LOGGER_BUFFER_SHM_NAME, logger->buffer);
	logger->buffer_shm = shm_open(shm_name, O_RDWR, LOGGER_FILES_MODE);
	assert(logger->buffer_shm  >= 0);
	logger->buffer_data = mmap(NULL, logger->buffers->buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, logger->buffer_shm, 0);
	assert(logger->buffer_data && logger->buffer_data != MAP_FAILED);
}

void unmap_buffer(logger_t* logger)
{	
	assert(logger->buffer_data);
	munmap(logger->buffer_data, logger->buffers->buffer_size);
	close(logger->buffer_shm);
	logger->buffer_shm = -1;
	logger->buffer_data = NULL;
}

void queue_log_buffer(buffer_queue_t* queue, log_buffer_t* buffers, int buffer)
{
	assert(buffers[buffer].in_use);
	assert(buffers[buffer].next == -1);
	buffers[buffer].in_use = false;
	
	sem_wait(queue->sem);

	if (*(queue->last) == -1)
	{
		assert(*(queue->first) == -1);
	
		*(queue->last) = *(queue->first) = buffer;
	}
	else
	{
		assert(buffers[*(queue->last)].next == -1);
	
		buffers[*(queue->last)].next = buffer;
		*(queue->last) = buffer;
	}

	buffers[buffer].next = -1;

	assert(!buffers[buffer].in_use);
	sem_post(queue->sem);
	sem_post(queue->available_sem);
}

int dequeue_log_buffer(buffer_queue_t* queue, log_buffer_t* buffers)
{
	sem_wait(queue->available_sem);
	sem_wait(queue->sem);
	
	assert(*(queue->first) != -1);
	
	int buffer = *(queue->first);
	*(queue->first) = buffers[buffer].next;
	
	if (*(queue->first) == -1)
		*(queue->last) = -1;
	
	sem_post(queue->sem);
	
	assert(!buffers[buffer].in_use);
	buffers[buffer].in_use = true;
	buffers[buffer].next = -1;
	
	return buffer;
}

void init_buffer_queues(logger_t* logger, bool server)
{
	int oflags = server ? LOGGER_CREATION_OFLAGS : 0;
	
	logger->full_buffers.sem           = sem_open(LOGGER_FULL_BUFFERS_SEM_NAME,           oflags, LOGGER_FILES_MODE, 1);
	logger->full_buffers.available_sem = sem_open(LOGGER_FULL_BUFFERS_AVAILABLE_SEM_NAME, oflags, LOGGER_FILES_MODE, 0);
	logger->free_buffers.sem           = sem_open(LOGGER_FREE_BUFFERS_SEM_NAME,           oflags, LOGGER_FILES_MODE, 1);
	logger->free_buffers.available_sem = sem_open(LOGGER_FREE_BUFFERS_AVAILABLE_SEM_NAME, oflags, LOGGER_FILES_MODE, 0);
	
	assert(logger->full_buffers.sem);
	assert(logger->full_buffers.available_sem);
	assert(logger->free_buffers.sem);
	assert(logger->free_buffers.available_sem);

	if (server)
	{
		logger->buffers->free_buffer_first = -1;
		logger->buffers->free_buffer_last  = -1;
		logger->buffers->full_buffer_first = -1;
		logger->buffers->full_buffer_last  = -1;
	}
	
	logger->free_buffers.first = &logger->buffers->free_buffer_first;
	logger->free_buffers.last  = &logger->buffers->free_buffer_last;
	logger->full_buffers.first = &logger->buffers->full_buffer_first;
	logger->full_buffers.last  = &logger->buffers->full_buffer_last;
}

void init_buffers(logger_t* logger, int n_buffers)
{
	int i;
	for (i = 0; i < n_buffers; i++)
	{
		logger->buffers->buffers[i].in_use = true;
		logger->buffers->buffers[i].next = -1;
		
		char shm_name[PATH_MAX];
		sprintf(shm_name, LOGGER_BUFFER_SHM_NAME, i);
		int buffer_shm = shm_open(shm_name, LOGGER_CREATION_OFLAGS | O_RDWR, LOGGER_FILES_MODE);
		assert(buffer_shm >= 0);
		ftruncate(buffer_shm, logger->buffers->buffer_size);
	
	/*
		logger->buffer = i;
		map_buffer(logger);
		logger->buffer = -1;
		logger->buffer_data = NULL;
		logger->buffer_shm = 0;
	*/
	
		
		//close(buffer_shm);
		queue_log_buffer(&logger->free_buffers, logger->buffers->buffers, i);
	
	}
}

void init_logger(logger_t* logger, bool server, bool force, int log_level, int n_buffers, size_t buffer_size)
{
	if (server && force)
		remove_files();
		
	logger->buffer = -1;
	logger->buffer_shm = -1;
	logger->buffer_data = NULL;
	
	int extra_oflags = server ? LOGGER_CREATION_OFLAGS : 0;
	
	logger->buffers_shm = shm_open(LOGGER_SHM_NAME, extra_oflags | O_RDWR, LOGGER_FILES_MODE);
	assert(logger->buffers_shm >= 0);
	
	size_t buffers_size = sizeof (logger_buffers_t) + sizeof (log_buffer_t) * n_buffers;	
	if (server)
		ftruncate(logger->buffers_shm, buffers_size);

	logger->buffers = mmap(NULL, buffers_size, PROT_READ | PROT_WRITE, MAP_SHARED, logger->buffers_shm, 0);
	assert(logger->buffers && logger->buffers != MAP_FAILED);

	init_buffer_queues(logger, server);
	
	if (server)
	{
		logger->buffers->buffer_size = buffer_size;
		logger->buffers->log_level = log_level;
		init_buffers(logger, n_buffers);
	}
	
	logger->curr_position = 0;
	logger->name[0] = 0;
	logger->initialized = true;
}

void remove_files()
{
	shm_unlink(LOGGER_SHM_NAME);
	sem_unlink(LOGGER_FULL_BUFFERS_SEM_NAME);
	sem_unlink(LOGGER_FULL_BUFFERS_AVAILABLE_SEM_NAME);
	sem_unlink(LOGGER_FREE_BUFFERS_SEM_NAME);
	sem_unlink(LOGGER_FREE_BUFFERS_AVAILABLE_SEM_NAME);
	
	int i = 0;
	char shm_name[PATH_MAX];
	do
	{
		sprintf(shm_name, LOGGER_BUFFER_SHM_NAME, i++);
	} while (!shm_unlink(shm_name));
}

