#include "logger.h"
#define SIZE 100
void *buffer_for_stack [SIZE];

logger_t* logger_init(char *filename) {
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

void logger_deinit() {
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
        created_logger -> buffer -> buffer[i] = NULL;
    }
    created_logger -> buffer -> num_of_circules = 0;
    return 0;
}

void buff_deinit() {
    return;
}

char* formulate_message(log_type_t log_type, char* message) {
    char* formulated_message = (char*)calloc(strlen(message) + strlen(log_types[2]) + 1, sizeof(char));
    strcat(formulated_message, log_types[log_type]);
    strcat(formulated_message, message);
    return formulated_message;
}

int write_log(log_type_t log_type, char *message) {
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

    if (log_type < Debug) {
        return -4;
    }

    if (log_type == Fatal) {
        int i = 0;
        if(created_logger -> buffer -> num_of_circules == 0) {
            for (i = 0; i < created_logger -> buffer -> position_of_last_empty; i++) {
                fputs(created_logger -> buffer -> buffer[i], created_logger -> file_pointer);
                free(created_logger -> buffer -> buffer[i]);
                created_logger -> buffer -> buffer[i] = NULL;

            }
        } else {
            for (i = 0; i < BUFFER_SIZE; i++) {
                fputs(created_logger -> buffer -> buffer[i], created_logger -> file_pointer);
                free(created_logger -> buffer -> buffer[i]);
                created_logger -> buffer -> buffer[i] = NULL;
            }
        }
        int nptrs = backtrace(buffer_for_stack, SIZE);
        char **stack_interior;
        stack_interior = backtrace_symbols(buffer_for_stack, SIZE);
        for(i = 0; i < nptrs; i++) {
            printf("%s\n",stack_interior[i]);
        }
        logger_deinit();
        return -5;
    }

    if(log_type < Fatal) {
        if(created_logger -> buffer -> position_of_last_empty == BUFFER_SIZE) {
            created_logger -> buffer -> position_of_last_empty = 0;
            created_logger -> buffer -> num_of_circules += 1;
        }
        if(created_logger -> buffer -> num_of_circules != 0) {
            free(created_logger -> buffer -> buffer[created_logger -> buffer-> position_of_last_empty]);
            created_logger -> buffer -> buffer[created_logger -> buffer-> position_of_last_empty] = NULL;
        }
        created_logger -> buffer -> buffer[created_logger -> buffer-> position_of_last_empty] = formulate_message(log_type, message);
        created_logger -> buffer -> position_of_last_empty +=1;
        return 0;
    }
}

int write_log_to_file(log_type_t log_type, char *message) {
    char *mes = formulate_message(log_type, message);
    fputs(mes, created_logger -> file_pointer);
    return 0;
}
