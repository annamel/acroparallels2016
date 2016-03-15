#ifndef LOGGER
#define LOGGER

#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_OF_BUFFER 1024
#define SIZE_STR_LOG 64
#define LOGGER_FILE "logger_file"
#define SIZE_CALL_TRACE 10

typedef enum log_option {
        INFO,
        WARNING,
        ERROR
} LogOption;

typedef enum Bool_enum {
        FALSE,
        TRUE
} Bool;

typedef struct log {
        Bool fatal;
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
int log_init(void) {
        return 0; 
}
int log_error(Bool fatal, LogOption option, const char *string) {
        return 0;
}
int log_end(void) {
        return 0;
}
#endif

#ifdef DEBUG

// глобальная переменная
CircleArray cir_array;

void init_of_array(void) {
        cir_array.first_free = 0;
        cir_array.end_array = 0;
}
Bool array_is_filled(void) {
        return (cir_array.end_array - cir_array.first_free == 1 ||
                (cir_array.first_free == SIZE_OF_BUFFER-1 && cir_array.end_array == 0)) ? TRUE : FALSE;
}
int print_in_array(Log log, Bool ignor) {
        if (array_is_filled() && ignor == FALSE){
                return NO_FREE_SPACE;
        }
        cir_array.array[cir_array.first_free] = log;
        if (++cir_array.first_free == SIZE_OF_BUFFER)
                cir_array.first_free = 0;
        if (cir_array.first_free == cir_array.end_array)
                if (++cir_array.end_array == SIZE_OF_BUFFER)
                        cir_array.end_array = 0;
        return 0;
}

void print_log_in_file(FILE *file, Log log) {
        #define NAME_AS_STRING(name) #name
        char *bool_name[] = {
                NAME_AS_STRING(FALSE),
                NAME_AS_STRING(TRUE)
        };
        char *option_name[] = {
                NAME_AS_STRING(DEBUG),
                NAME_AS_STRING(INFO),
                NAME_AS_STRING(WARNING),
                NAME_AS_STRING(ERROR)
        };
        fprintf(file, "FATAL = %s\n",bool_name[log.fatal]);
        fprintf(file, "OPTION = %s\n", option_name[log.option]);
        fprintf(file, "%s\n", log.description);
}

int print_in_file(void) {
        if (cir_array.first_free == cir_array.end_array)
                return 0;
        FILE *file = fopen(LOGGER_FILE, "a");
        if (!file) {
                return CAN_NOT_OPEN_FILE;
        }
        int i = cir_array.end_array;
        if (cir_array.first_free - cir_array.end_array > 0) {
                for (; i < cir_array.first_free; i++)
                        print_log_in_file(file, cir_array.array[i]);
        } else {
                for (;i < SIZE_OF_BUFFER; i++)
                        print_log_in_file(file, cir_array.array[i]);
                for (i = 0; i < cir_array.first_free; i++)
                        print_log_in_file(file, cir_array.array[i]);
        }
        fclose(file);
        init_of_array();
        return 0;
}
int log_init(void) {
        init_of_array();
        FILE *file = fopen(LOGGER_FILE, "w");
        if (!file)
                return CAN_NOT_OPEN_FILE;
        fclose(file);
        return 0;
}
int print_trace(void) {
        void *arr[SIZE_CALL_TRACE];
        size_t size = backtrace(arr, SIZE_CALL_TRACE);
        backtrace_symbols_fd(arr, size, 2);
        return 0;
}
int log_error(Bool fatal, LogOption option, const char *string) {
        Log log;
        log.fatal = fatal;
        log.option = option;
        strncpy(log.description, string, SIZE_STR_LOG);
        
        int err1 = 0;
        if (print_in_array(log, FALSE) == NO_FREE_SPACE){
                err1 = print_in_file();
                print_in_array(log, TRUE);
        }
        if (log.fatal == TRUE) {
                print_in_file();
                print_trace();
        }
        if (err1 != 0)
                return -1;
        return 0;
}
int log_end(void) {
        return print_in_file();
}
#endif
#endif







