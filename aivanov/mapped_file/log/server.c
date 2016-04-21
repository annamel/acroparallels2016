#include "logger.h"
#include <stdlib.h>
#include <errno.h>

void run_server(int log_level, int n_buffers, int buffer_size)
{
	logger_t logger;
	init_logger(&logger, true, true, log_level, n_buffers, buffer_size);
	
	while (true)
	{
		if (logger.buffer != -1)
		{
			unmap_buffer(&logger);
			queue_log_buffer(&logger.free_buffers, logger.buffers->buffers, logger.buffer);
		}
			
		logger.buffer = dequeue_log_buffer(&logger.full_buffers, logger.buffers->buffers);
		map_buffer(&logger);
		printf("%s\n", logger.buffer_data);	
	}
}	

void print_usage(const char* error, const char* pname)
{
	fprintf(stderr, "%s\n"
					"Usage: %s <debug level> <number of buffers> <buffer size>\n"
					"\tPossible debug levels are: I(info), W(warning), D(debug), V(verbose), E(error)\n",
					error, pname);
					
	exit(1);
}

int main(int argc, char** argv)
{
	if (argc != 4)
		print_usage("Invalid arguments.", argv[0]);
	
	if (argv[1][1])
		print_usage("Logging mode too long.", argv[0]);
	
	int log_level = 0;
	switch (argv[1][0])
	{
		case 'I':
			log_level = LOGGER_LEVEL_INFO;
			break;
		case 'W':
			log_level = LOGGER_LEVEL_WARNING;
			break;
		case 'D':
			log_level = LOGGER_LEVEL_DEBUG;
			break;
		case 'V':
			log_level = LOGGER_LEVEL_VERBOSE;
			break;
		case 'E':
			log_level = LOGGER_LEVEL_ERROR;
			break;
		default:
			print_usage("Invalid logging level.", argv[0]);
	}
	
	int n_buffers = atoi(argv[2]);
	if (errno || n_buffers < 0)
		print_usage("Invalid number of buffers.", argv[0]);
	
	int buffer_size = atoi(argv[3]);
	if (errno || buffer_size < 0)
		print_usage("Invalid buffer size.", argv[0]);
	
	run_server(log_level, n_buffers, buffer_size);
	
	return 0;
}

