//*********************************************************
//********                                       **********
//********      Created by Alexander Snorkin     **********
//********              22.04.2016               **********
//********                                       **********
//*********************************************************
#ifndef LOGGER_H
#define LOGGER_H



#include <stdio.h>



#define BUFF_SIZE 256
#define DEFAULT_LOGFILE "logfile.txt"
#define DEFAULT_LOGLEVEL FATAL
#define CALL_STACK_SIZE 256



typedef enum log_level log_lvl_t;
typedef struct logger logger_t;
typedef struct ring_buffer ring_buff_t;



//  Enumeration of levels of logging
enum log_level
{
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    COUNT_OF_LOGLEVELS
};



//  The logger structure
//  Contains the logfile pointer, ring buffer pointer
//  for log buffering and default log level
struct logger
{
    FILE *file;
    ring_buff_t *buffer;
    log_lvl_t default_log_level;
};



//  The ring buffer structure
//
struct ring_buffer
{
    char *buffer[BUFF_SIZE];
    unsigned int head;
    unsigned int tail;
};



//  This func initialize the logger
//  Logger realized as a Singletone pattern
//
//  - ARGUMENTS
//      filename - the logfile name. If it is
//                 NULL default logfile will use
//
//  - RETURNED VALUE
//      if func calls first time => new logger pointer
//      else => LOGGER_SINGLETONE pointer
//      something goes wrong => NULL
logger_t *logger_init();



//  This func writes log directly in the logfile without buffer
//  It recommends to use this func only in critical cases
//  (!!!) If log_level is FATAL program will exit with code -1
//  and flush its buffer into the file
//  - ARGUMENTS
//      message - message to write
//      log_level - the log level of message
//
//  - RETURNED VALUE
//      all is good => 0
//      logger init has failed => EAGAIN
//      message pointer is NULL => EINVAL
//      log_level more than max log level => EINVAL
//      log_level less than default log level => EINVAL
//      log_level is FATAL => exit with EXIT_FAILURE code
int log_write_in_logfile(log_lvl_t log_level, const char *__restrict message, ...);



//  This func writes the log message
//  (!!!) If log_level is FATAL program will exit with code -1
//  and flush its buffer into the file
//  - ARGUMENTS
//      message - message to write
//      log_level - the log level of message
//
//  - RETURNED VALUE
//      all is good => 0
//      logger init has failed => EAGAIN
//      message pointer is NULL => EINVAL
//      log_level more than max log level => EINVAL
//      log_level less than default log level => EINVAL
//      log_level is FATAL => exit with EXIT_FAILURE code
int log_write(log_lvl_t log_level, const char *__restrict message, ...);




//  This func deinitialize the logger
//  and frees the memory
//  - RETURNED VALUE
//      all is good => 0
//      can't flush buffer => ETXTBSY
//      can't close the file => ETXTBSY
int logger_deinit();



//  This func sets the default log level
//  - ARGUMENTS
//      new_default_loglvl - new default log level for set
//
//  - RETURNED VALUE
//      all is good => 0
//      logger init has failed => EAGAIN
//      new_default_loglvl more than max log level => EINVAL
int log_set_default_loglvl(log_lvl_t new_default_loglvl);



//  This func sets the logfile
//  - ARGUMENTS
//      filename - the path of setting logfile
//  - RETURNED VALUE
//      all is good => 0
//      logger init has failed => EAGAIN
//      filename pointer is NULL => EINVAL
//      can't close current logfile => ETXTBSY
//      can't open new logfile =>
int log_set_logfile(const char *filename);



#endif // LOGGER_H
