//*********************************************************
//********                                       **********
//********      Created by Alexander Snorkin     **********
//********              22.03.2016               **********
//********                                       **********
//*********************************************************
#ifndef LOGGER_H
#define LOGGER_H


#define BUFF_SIZE 1024
#define DEFAULT_LOGFILE "C://logfile.log"
#define DEFAULT_LOGLEVEL DEBUG


#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>


//  Enumeration of levels of logging
typedef enum log_level
{
    DEBUG = 0,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    COUNT_OF_LOGLEVELS
} log_lvl_t;


//  String prefix of logging levels for log messages
const char *LOG_LEVELS[COUNT_OF_LOGLEVELS] = {"DEBUG::",
                                              "INFO::",
                                              "WARNING::",
                                              "ERROR::",
                                              "FATAL::"};



//  The ring buffer structure
//  Contains the buffer of log messages,
//  numbers of the head and tail
//  Head is ...blablabla...
typedef struct ring_buffer
{
    char *buffer[BUFF_SIZE];
    unsigned short head;
    unsigned short tail;
} ring_buff_t;


//  The logger structure
//  Contains the logfile pointer, ring buffer pointer
//  for log buffering and default log level
typedef struct logger
{
    FILE *file;
    ring_buff_t *buffer;
    log_lvl_t default_log_level;
} logger_t;

//************************************************
//**  GLOBAL VARIABLE !!! SINGLETON !!! LOGGER ***
//**                                           ***
         logger_t *LOGGER_SINGLETON = NULL;    //*
//************************************************



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
logger_t *logger_init(char *filename);



//  This func deinitialize the logger
//  and frees the memory
//  - RETURNED VALUE
//      no value
void logger_deinit();



//  This func sets the default log level
//  - ARGUMENTS
//      new_default_loglvl - new default log level for set
//
//  - RETURNED VALUE
//      all is good => 0
//      LOGGER_SINGLETON pointer is NULL => -1
//      new_default_loglvl more than max log level => -2
int set_default_log_level(log_lvl_t new_default_loglvl);



//  This func sets the logfile
//  - ARGUMENTS
//      filename - the path of setting logfile
//  - RETURNED VALUE
//      all is good => 0
//      LOGGER_SINGLETON pointer is NULL => -1
//      filename pointer is NULL => -2
//      can't close current logfile => -3
//      can't open new logfile => -4
int set_logfile(char *filename);



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
//      LOGGER_SINGLETON pointer is NULL => -1
//      message pointer is NULL => -2
//      log_level more than max log level => -3
//      log_level less than default log level => -4
int log_write_in_logfile(char *message, log_lvl_t log_level);



//  This func writes the log message
//  (!!!) If log_level is FATAL program will exit with code -1
//  and flush its buffer into the file
//  - ARGUMENTS
//      message - message to write
//      log_level - the log level of message
//
//  - RETURNED VALUE
//      all is good => 0
//      LOGGER_SINGLETON pointer is NULL => -1
//      message pointer is NULL => -2
//      log_level more than max log level => -3
//      log_level less than default log level => -4
int log_write(char *message, log_lvl_t log_level);



//  This func initialize the buffer
//  and fills it by nills
//  - RETURNED VALUE
//      all is good => 0
//      LOGGER_SINGLETON is NULL => -1
//      calloc can't allocate memory for buffer => -2
int buff_init();



//  This func checks filling of the buffer
//  - RETURNED VALUE
//      buffer is full => 1
//      buffer is not full => 0
//      LOGGER_SINGLETON is NULL => -1
int buff_is_full();



//  This func creates the log message by message and log level
//  It frees the message pointer
//  (!!!) IT FUNCTION DOESN'T CHECK THE ARGUMENTS (!!!)
//  it uses only in places where arguments are good
//  - ARGUMENTS
//      message - message to wrap
//      log_level - the log level of message
//
//  - RETURNED VALUE
//      all is good => wrapped message pointer
char *create_message(char *message, log_lvl_t log_level);



#endif // LOGGER_H
