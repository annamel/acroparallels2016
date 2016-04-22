#include "logger.h"

logger_t* init(char *filename) {
	if(created_logger != NULL) {
		return created_logger;
	}
	created_logger = (logger_t*)calloc(1, sizeof(logger_t));
	if(created_logger == NULL) {
		perror("Initialization of logger failed!\n");
		return NULL;
	}
	if(filename == NULL) {
		filename = LOG_FILE_BY_DEFAULT;
	}
	created_logger -> file_pointer = fopen(filename, "w");
	if((created_logger -> file_pointer) == NULL) {
		perror("File initialiation failed!\n");
		free(created_logger);
		return NULL;
	}
	if(buff_init() == -2) {
		perror("Initialization of logger failed!\n");
		return NULL;
	}
	created_logger -> type_of_log_by_default = Debug;
	return created_logger;
}

void deinit() {
	if(created_logger == NULL) {
		printf("Logger hasn't been initiated!\n");
		return;
	}
	fclose(created_logger -> file_pointer);
	buff_deinit();
	free(created_logger);
	return;
}

int buff_init() {
	if(created_logger == NULL) {
		perror("Logger is not initialized, so buffer initialization is failed!\n");
		return -1;
	}
	created_logger -> buffer = (buffer_t*)calloc(1, sizeof(buffer_t));
	if((created_logger -> buffer) == NULL) {
		perror("The initialization of buffer is failed!\n");
		fclose(created_logger -> file_pointer);
		free(created_logger);
		return -2;
		}	
	created_logger -> buffer -> position_of_last_empty = 0;
	created_logger -> buffer -> end = 0;
	int i = 0;
	for(i = 0; i < BUFFER_SIZE; i++) {
		created_logger -> buffer -> buffer[i] = "";
	}
	return 0;
}

void buff_deinit() {
	return
}

char* formulate_message(char* message, log_type_t log_type) {
	char* formulated_message = (char*)calloc(strlen(message) + strlen(log_types[2]) + 1, sizeof(char));
	strcat(formulated_message, log_types[log_type]);
	strcat(formulated_message, message);
	return formulated_message;
}

int write_log(char *message, log_type_t log_type) {
	if (created_logger == NULL) {
		perror("Logger is not initialized! There is not possible to write a message!\n");
		return -1;
	} 

	if (message == NULL) {
		errno = EINVAL;
		perror("It is impossible to set file! Input message is wrong!\n");
		return -2;
	}

	if (log_type > Fatal) {
		perror("Log level is wrong!\n");
		return -3;
	}

	if (log_type < created_logger -> type_of_log_by_default) {
		return -4;
	}

	if (log_type == Fatal) {
		int i = 0;
		for (i = 0; i < BUFFER_SIZE - 1; i++) {
			fputs(created_logger -> buffer -> buffer[i], created_logger -> file_pointer);
			free(created_logger -> buffer -> buffer[i]);
			created_logger -> buffer -> buffer[i] = "";
		}
		int nptrs = backtrace(buffer_for_stack, SIZE);
		char **stack_interior;
		stack_interior = backtrace_symbols(buffer_for_stack, SIZE);
		for(i = 0; i < nptrs; i++) {
			printf("%s\n",stack_interior[i]);
		}
		deinit();
		exit(-1);
	}
	
	if(log_type < Fatal) {
		if(created_logger -> buffer -> position_of_last_empty == BUFFER_SIZE) {
			created_logger -> buffer -> position_of_last_empty = 0;
		}
		free(created_logger -> buffer -> buffer[created_logger -> buffer-> position_of_last_empty]);
		created_logger -> buffer -> buffer[created_logger -> buffer-> position_of_last_empty] = "";
		created_logger -> buffer -> buffer[created_logger -> buffer-> position_of_last_empty] = formulate_message(message, log_type);
		created_logger -> buffer -> position_of_last_empty +=1;
		return 0;
	}
}




