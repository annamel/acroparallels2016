#include "logger.h"


inline void switch_buf(struct Logger* logger)
{
	if (logger->cur_buf == logger->buf_assoc - 1)
		logger->cur_buf = 0;
	else
		logger->cur_buf += 1;
	write_p = (char*)log_buffer[logger->cur_buf];
}

void log_init(log_level_t log_stage, char* str, assoc_of_buf_t associativity) 
{
	logger = malloc(sizeof(struct Logger));
	filedesc = open(str, O_RDWR | O_CREAT, S_IRWXU);
	logger->log_file = fdopen(filedesc, "r+");
	logger->level = log_stage;
	logger->buf_assoc = TWO_WAY;

	if (associativity ==  FOUR_WAY 
		|| associativity ==  EIGHT_WAY)
		logger->buf_assoc = associativity;

	logger->size_of_buf = BUFFER_SIZE / logger->buf_assoc;
	log_buffer = malloc(sizeof(char*) * logger->buf_assoc); 
	int i = 0;

	while (i < logger->buf_assoc)
	{
		log_buffer[i] = (char*)malloc(sizeof(char) * BUFFER_SIZE);
		i++;
	}

	logger->cur_buf = 0;
	write_p = (char*)log_buffer[logger->cur_buf];	
}

inline void stall_cur_buf()
{
	*write_p = '\0';
}

int stall_buf(struct Logger* logger, int buf_id)
{
	memset((char*)log_buffer[buf_id], '\0', logger->size_of_buf);   
	return 0;
}	

int flush_buf(struct Logger* logger)
{
	int i = 0;
	
	for (i = logger->cur_buf + 1; i < logger->buf_assoc; i++)
	{	
		fprintf(logger->log_file,"%s", (char*)log_buffer[i]);
		stall_buf(logger, i);
	}
	
	for (i = 0; i < logger->cur_buf; i++)
	{	
		fprintf(logger->log_file, "%s", (char*)log_buffer[i]);
		stall_buf(logger, i);
	}
	
	fprintf(logger->log_file, "%s", (char*)log_buffer[logger->cur_buf]);
	logger->cur_buf = 0;
	write_p = (char*)log_buffer[logger->cur_buf];
	
	return 0;
}	

void log_deinit(struct Logger* logger)
{
	int i = 0;
	 
	for (i = 0; i < logger->buf_assoc; i++)
	{	
		free(log_buffer[i]);
	}
	free(log_buffer);
	 	
	free(logger);
	close(filedesc);
	fclose(logger->log_file);
}


