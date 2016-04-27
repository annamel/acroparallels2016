/*

 # logger header file
 #
 # Lang:     C
 # Author:   okhlopkov
 # Version:  0.3

 */

#include <stdlib.h>
#include <stdint.h>

// EXAMPLE: "sources/logger"
#define PATH_TO_LOGGER_FOLDER "sources/logger"

#define LOG(...) \
            do { if (logger == NULL || msg == NULL) logger = logger_init(); \
              sprintf(msg, __VA_ARGS__);  \
            logger_log(logger, msg);} while (0)

#define BUFFERSIZE 2048
#define MSGSIZE 256

typedef struct {
       char *data;
       uint32_t size;
} buffer_t;

typedef struct {
        char *log_file_name;
        buffer_t *buffer;
} logger_t;

logger_t *logger;
char *msg;


/* Constructor & Destructor */
logger_t *logger_init();
uint32_t logger_deinit(logger_t *logger);

/* Logs string */
uint32_t logger_log(logger_t *logger, char *string);
