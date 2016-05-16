//
//  logger.h
//  par_mapped_file
//
//  Created by IVAN MATVEEV on 16.05.16.
//  Copyright © 2016 IVAN MATVEEV. All rights reserved.
//

#ifndef logger_h
#define logger_h

#include <stdio.h>

#define SIZE_OF_BUFFER 1024
#define SIZE_STR_LOG 64

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

int LOG( LogOption option, const char *string );
void set_log_level( LogOption option );
void set_log_file( const char * path );

#endif /* logger_h */
