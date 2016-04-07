enum LogLevel {
    INFO = 0,
    WARNING,
    ERROR,
    DEBUG
};

//Some thread-safe little logger
//
//Compile with -std=gnu99
//Before start, you must define one of LOG_LEVEL_3 LOG_LEVEL_2 LOG_LEVEL_1 LOG_LEVEL_0
//In case of selecting high log level all lessers levels automatically included.
//Othserwise none of code included
//In case of

void lilog(int loglevel, char* str, ...);
void lilog_finish(char** argv);
#define log_cond(CONDITION)
#define log_cond_msg(CONDITION, MSG, ...)
#define _if(CONDITION) if
void lilog_print_stack();

//lilog and lilog_finish avaliable in LOG_LEVEL_1
//log_cond in LOG_LEVEL_2. That macro logs if condition inside is failed. Error level is WARN
//log_cond_msg in LOG_LEVEL_3. That macro logs yours specified string if condition is failed. Error level is ERROR
//_if avaliable in LOG_LEVEL_3. It logs if failed condition. If BACKTRACE_ENABLE is NOT 0, it print backtrace in a log.
//all _if is equal to usual if in case of LOG_LEVEL_3 not defined. Error level is DEBUG
//lilog_print_stack() avaliable in LOG_LEVEL_3. Error level is DEBUG


#if defined(LOG_LEVEL_3) || defined(LOG_LEVEL_2) || defined(LOG_LEVEL_1) || defined(LOG_LEVEL_0)

    #ifndef MAX_BUFF
        #define MAX_BUFF 512
    #endif // MAX_BUFF

    #ifndef BUFF_COUNT
        #define BUFF_COUNT 128
    #endif // BUFF_COUNT

    #ifndef FLUSH_COUNT
        #define FLUSH_COUNT 32
    #endif // FLUSH_COUNT

    #include "log_level_0.h"

#endif

#if defined(LOG_LEVEL_3) || defined(LOG_LEVEL_2) || defined(LOG_LEVEL_1)
    #undef log_cond
    #include "log_level_1.h"
#endif

#if defined(LOG_LEVEL_3) || defined(LOG_LEVEL_2)
    #undef log_cond_msg
    #include "log_level_2.h"
#endif

#ifdef LOG_LEVEL_3

    #undef _if

    #ifndef BACKTRACE_ENABLE
    #define BACKTRACE_ENABLE 1
    #endif // BACKTRACE_ENABLE

    #include "log_level_3.h"

#endif



