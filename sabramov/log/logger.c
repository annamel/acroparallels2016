#include "logger.h"


inline void stall_cur_buf()
{
	*write_p = '\0';
}

int stall_buf(struct Logger* logger, int buf_id)
{
	memset((char*)log_buffer, '\0', BUFFER_SIZE);   
	return 0;
}	

int log_file_config(char* file_name)
{
	int len = strlen(file_name);
	conf_log_file = malloc(sizeof(char) * len);
	memcpy(conf_log_file, file_name, len);
	return 0;
}

void flush_buf()
{
	fprintf(logger->log_file, "%s", (char*)log_buffer);	
	write_p = (char*)log_buffer;
}


