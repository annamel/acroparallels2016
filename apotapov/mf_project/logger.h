//	Made by Artem Potapov
//	artem.potapov@frtk.ru

#ifndef logger_h
#define logger_h

#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100
#define LOG_FILE_BY_DEFAULT "log.txt"
#define AMOUNT_OF_OPTIONS 5


typedef enum condition 
{
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
	char* buffer[BUFFER_SIZE];
} buffer_t;

typedef struct logger
{
    FILE *file_pointer;
    buffer_t *buffer;
    log_type_t type_of_log_by_default;
} logger_t;

const char *log_types[AMOUNT_OF_OPTIONS] = {"DEBUG:: ",
                                       		"INFO:: ",
                                       		"WARNING:: ",
                                       		"ERROR:: ",
                                       		"FATAL:: "};

logger_t *created_logger = NULL;

logger_t* init(char *filename);
int buff_init();
void deinit();
int full_buff();
char* formulate_message(char *message, log_type_t log_type);
int write_log(char *message, log_type_t log_type);
int write_log_in_file(char* message, log_type_t log_type);
void buff_deinit();

#endif