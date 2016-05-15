#ifndef LOGGER
#define LOGGER

#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SIZE_OF_BUFFER 1024
#define SIZE_STR_LOG 64
#define SIZE_CALL_TRACE 10
#define SIZE_PATH_OF_FILE 256

char LOGGER_FILE[SIZE_PATH_OF_FILE] = "logger_file";

typedef enum log_option {
        debug,
        info,
        warning,
        error,
        fatal
} LogOption;

typedef enum bool_enam {
        FALSE,
        TRUE
} Bool;

typedef struct log {
        LogOption option;
        char description[SIZE_STR_LOG];
} Log;

typedef struct circle_array {
        Log array[SIZE_OF_BUFFER];
        unsigned int first_free;
        unsigned int end_array;
} CircleArray;

enum Errors {
        NO_FREE_SPACE = -1,
        CAN_NOT_OPEN_FILE = -2
};
// не влияем на производительность
#ifndef DEBUG
int LOG( LogOption option, const char *string ) {
        return 0;
}
void set_log_level( LogOption option ) {}
void set_log_file( const char * path ) {}
#endif

#ifdef DEBUG

void init_of_array( CircleArray *cir_array ) {
        cir_array->first_free = 0;
        cir_array->end_array = 0;
}
Bool array_is_filled( CircleArray *cir_array) {
        return (cir_array->end_array - cir_array->first_free == 1 ||
                (cir_array->first_free == SIZE_OF_BUFFER-1 && cir_array->end_array == 0)) ? TRUE : FALSE;
}
int print_in_array(CircleArray *cir_array, Log log, Bool ignor) {
        if (array_is_filled(cir_array) && ignor == FALSE){
                return NO_FREE_SPACE;
        }
        cir_array->array[cir_array->first_free] = log;
        if (++cir_array->first_free == SIZE_OF_BUFFER)
                cir_array->first_free = 0;
        if (cir_array->first_free == cir_array->end_array)
                if (++cir_array->end_array == SIZE_OF_BUFFER)
                        cir_array->end_array = 0;
        return 0;
}

void print_log_in_file(FILE *file, Log log) {
        #define NAME_AS_STRING(name) #name
        char *option_name[] = {
                NAME_AS_STRING(debug),
                NAME_AS_STRING(info),
                NAME_AS_STRING(warning),
                NAME_AS_STRING(error),
                NAME_AS_STRING(fatal),
        };
        fprintf(file, "%s:\t", option_name[log.option]);
        fprintf(file, "%s\n", log.description);
}

int print_in_file( CircleArray *cir_array ) {
        if (cir_array->first_free == cir_array->end_array)
                return 0;
        FILE *file = fopen(LOGGER_FILE, "a");
        if (!file) {
                printf("error: can't open file: %s\n", LOGGER_FILE);
                return CAN_NOT_OPEN_FILE;
        }
        int i = cir_array->end_array;
        if (cir_array->first_free - cir_array->end_array > 0) {
                for (; i < cir_array->first_free; i++)
                        print_log_in_file(file, cir_array->array[i]);
        } else {
                for (;i < SIZE_OF_BUFFER; i++)
                        print_log_in_file(file, cir_array->array[i]);
                for (i = 0; i < cir_array->first_free; i++)
                        print_log_in_file(file, cir_array->array[i]);
        }
        fclose(file);
        init_of_array( cir_array );
        return 0;
}
int log_init(CircleArray *cir_array) {
        init_of_array(cir_array);
        FILE *file = fopen(LOGGER_FILE, "w");
        if (!file)
                return CAN_NOT_OPEN_FILE;
        fclose(file);
        return 0;
}
int print_trace(CircleArray *cir_array) {
        int fd = open( LOGGER_FILE, O_APPEND | O_WRONLY);
        void *arr[SIZE_CALL_TRACE];      
        size_t size = backtrace(arr, SIZE_CALL_TRACE);
        backtrace_symbols_fd(arr, size, fd);
        close(fd);
        return 0;
}
CircleArray *global_ptr_on_buf; // неизбежная глобальная переменная
LogOption log_level = debug;
long long unsigned int quantity_log = 0;

void log_deinit(void) {
        print_in_file(global_ptr_on_buf);
}

int LOG( LogOption option, const char *string ) {
        static CircleArray buffer;
        if (quantity_log == 0) {
                log_init(&buffer);
                global_ptr_on_buf = &buffer;
                atexit(log_deinit);
        }
        quantity_log++;

        if (option < log_level) 
                return 0;

        Log log;
        log.option = option;
        strncpy(log.description, string, SIZE_STR_LOG);
        
        int err = 0;
        if (print_in_array(&buffer, log, FALSE) == NO_FREE_SPACE){
                err = print_in_file(&buffer);
                print_in_array(&buffer, log, TRUE);
        }
        if (log.option == fatal) {
                print_in_file(&buffer);
                print_trace(&buffer);
        }
        return err;
}

void set_log_level( LogOption option ) {
        log_level = option;
}

void set_log_file( const char * path) {
        if (quantity_log > 0)
                print_in_file(global_ptr_on_buf);
        quantity_log = 0;
        strncpy(LOGGER_FILE, path, SIZE_PATH_OF_FILE);
}
#endif
#endif







