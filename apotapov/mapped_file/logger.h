#ifndef logger
#define logger

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 20

typedef enum condition {
	Debug = 0, 
	Info, 
	Warning, 
	Error,
	Fatal
} log_type_t;

typedef struct circuit_buffer 
{
	unsigned int position_of_last_empty;
	unsigned int end;
	unsigned int num_of_circules;
	char* buffer[BUFFER_SIZE];
} buffer_t;

typedef struct logger
{
    FILE *file_pointer;
    buffer_t *buffer;
    log_type_t type_of_log_by_default;
} logger_t;

logger_t *created_logger;

logger_t* logger_init(char *filename);
void logger_deinit();
int buff_init();
void buff_deinit();
char* formulate_message(log_type_t log_type, char* message);
int write_log(log_type_t log_type, char *message);
int write_log_to_file(log_type_t log_type, char *message);

#endif 

