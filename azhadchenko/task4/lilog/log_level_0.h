//Next defines represent not clear shitty lov-level hardcoded solution of problem
//when someone tryes to flush chunk of buffers, while some of the buffers still not fromed yet
//For this reason, there are allocated 1 more control byte at the end of each buffer
//In formed state that byte is 0, so it wont affect any string inside buffer

#define TRUE_BUFF_SIZE (MAX_BUFF + 1)
#define GET_CONTROL_BIT(index)  (TRUE_BUFF_SIZE * (index + 1) - 1)

#define BUSY_BUFFER 1
#define WRITTEN_BUFFER 0

struct Logger {
    char* ring;
    unsigned int index;
    unsigned int printed_until;
    unsigned int last_free;
    int output_fd;
    unsigned int flush_busy;
} logger = {0};


unsigned int get_index() {
    int old_index = 0, index = 0, last_free = 0;

    while(1) {
        old_index = logger.index;
        last_free = logger.last_free;
        index = (old_index + 1) % BUFF_COUNT;

        if(old_index == last_free)
            continue;

        if(__sync_bool_compare_and_swap(&(logger.index), old_index, index))
            break;
    }

    return old_index;
}

#define KNRM  "\x1B[0m["
#define KRED  "\x1B[31m["
#define KGRN  "\x1B[32m["
#define KYEL  "\x1B[33m["
#define KBLU  "\x1B[34m["
#define KMAG  "\x1B[35m["
#define KCYN  "\x1B[36m["
#define KWHT  "\x1B[37m["
#define RESET "] \033[0m"

#define GET_MODE_STR(MODE) #MODE

char* get_mode(int mode) {
    switch(mode) {
    case INFO :
        return KWHT GET_MODE_STR(INFO) RESET "   ";
    case WARNING :
        return KBLU GET_MODE_STR(WARNING) RESET;
    case ERROR :
        return KYEL GET_MODE_STR(ERROR) RESET "  ";
    case DEBUG :
        return KRED GET_MODE_STR(DEBUG) RESET "  ";
    default :
        return KMAG GET_MODE_STR(UNKNOWN) RESET;
    }
}

char* prepare_log(int message_level, unsigned int index, int* left_len) {
    size_t offset = index * TRUE_BUFF_SIZE;
    time_t t = {0};
    unsigned int written = 0;

    time(&t); //need to profile it

    written = snprintf((logger.ring + offset), MAX_BUFF, "%s%.24s: ", get_mode(message_level), ctime(&t));
    //Whoops, 24 is hardcoded - its the length of ctime() without \n\0

    *left_len = MAX_BUFF - written;

    return (logger.ring + offset + written);
}

void lilog_flush(size_t start, size_t finish);

#define FORCE_WRITE     1
#define AUTOMATIC_WRITE 0
#define FLUSH_WAIT 10000000

void append_log(unsigned int index, int force) {
    int avaliable = 0;
    int old_count = logger.printed_until;
    int new_count = 0;
    int last_given = logger.index;

#define CALC_AVALIBALE()    (index >= old_count) ? index - old_count : \
                            BUFF_COUNT - old_count + index

#define InRange \
  ( (last_given > old_count) ?  (index > old_count && index < last_given) : \
                                (index > old_count || index < last_given) )

    do {

        old_count = logger.printed_until;
        avaliable = CALC_AVALIBALE();
        last_given = logger.index;


        if(!((avaliable > FLUSH_COUNT) || force))
            return;

        if(!InRange)
            return;

    } while(!__sync_bool_compare_and_swap(&(logger.flush_busy), 0, 1));

    old_count = logger.printed_until;
    avaliable = CALC_AVALIBALE();
    last_given = logger.index;

    if(!((avaliable > FLUSH_COUNT) || force)) {
        logger.flush_busy = 0;
        return;
    }

    if(!InRange) {
        logger.flush_busy = 0;
        return;
    }

    new_count = (avaliable + old_count) % BUFF_COUNT;
    logger.printed_until = new_count;

    lilog_flush(old_count, new_count);
}


void lilog_flush(size_t start, size_t until) {
    size_t sz = 0;
    int err = 0;

    char* FLUSH_BUFF = (char*)calloc(1, MAX_BUFF * BUFF_COUNT);
    if(!FLUSH_BUFF) {
        fprintf(stderr, "Unable to allocate buffer for flushing logs. Consider logs lost");
        logger.flush_busy = 0;
        return;
    }


    if(start <= until) {
        for(size_t i = start; i < until; i++) {
            while(logger.ring[GET_CONTROL_BIT(i)] == BUSY_BUFFER);   //Little spin-lock. Flushing func waits all buffers to be written


            logger.ring[GET_CONTROL_BIT(i)] = BUSY_BUFFER;

            sz += snprintf(FLUSH_BUFF + sz, MAX_BUFF,
                             logger.ring + i * TRUE_BUFF_SIZE);
        }
    }
    else {
        for(size_t i = start; i < BUFF_COUNT; i++) {
            while(logger.ring[GET_CONTROL_BIT(i)] == BUSY_BUFFER);   //Little spin-lock. Flushing func waits all buffers to be written


            logger.ring[GET_CONTROL_BIT(i)] = BUSY_BUFFER;

            sz += snprintf(FLUSH_BUFF + sz, MAX_BUFF,
                             logger.ring + i * TRUE_BUFF_SIZE);
        }


        for(size_t i = 0; i < until; i++) {
            while(logger.ring[GET_CONTROL_BIT(i)] == BUSY_BUFFER);   //Little spin-lock. Flushing func waits all buffers to be written


            logger.ring[GET_CONTROL_BIT(i)] = BUSY_BUFFER;

            sz += snprintf(FLUSH_BUFF + sz, MAX_BUFF,
                             logger.ring + i * TRUE_BUFF_SIZE);
        }
    }

    err = write(logger.output_fd, FLUSH_BUFF, sz);
    if(err == -1)
        fprintf(stderr, "Attached logfile is dead. Unable to write into it\n");


    logger.last_free = until;
    logger.flush_busy = 0;
    free(FLUSH_BUFF);
}

void lilog(int loglevel, char* str, ...) {

    if(logger.ring == NULL)
        return;

    int left_len = 0;
    int written = 0;
    va_list ap;
    unsigned int index = get_index();
    char* tmp = prepare_log(loglevel, index, &left_len);

    va_start(ap, str);
    written = vsnprintf(tmp, left_len, str, ap);
    va_end(ap);

    if((written > 0) && (left_len - written > 0))
        snprintf(tmp + (size_t)written, left_len - written, "\n");

    logger.ring[GET_CONTROL_BIT(index)] = WRITTEN_BUFFER;

    append_log(index, AUTOMATIC_WRITE);
;}


void hello_msg(char** argv) {
    char buff[MAX_BUFF] = {0};
    snprintf(buff, MAX_BUFF,  "Process #%d, named %s started logging", getpid(), argv[0]);
    lilog(INFO, buff);
}

#define DEFAULT_PATH "/tmp"

void init_logger(char** argv, char* path){
    char BUFF[MAX_BUFF] = {0};

    logger.last_free = BUFF_COUNT - 1;

    logger.ring = (char*)calloc(sizeof(char), TRUE_BUFF_SIZE * BUFF_COUNT);
    if(logger.ring == NULL) {
        fprintf(stderr, "Unable to allocate buffer. Out of memory. Logging disabled \n");
        return;
    }


    for(int i = 0; i < BUFF_COUNT; logger.ring[GET_CONTROL_BIT(i++)] = BUSY_BUFFER);

    if(path) {
        logger.output_fd = open(path, O_WRONLY | O_CREAT | O_APPEND, 00666);
        if(logger.output_fd == -1) {
            fprintf(stderr, "Cannot open logfile. Trying to write into standart logfile\n");
        } else {
            hello_msg(argv);
            return;
        }
    }

    snprintf(BUFF, MAX_BUFF, DEFAULT_PATH "/%s.%d.log", argv[0] + 2, getpid());

    logger.output_fd = open(BUFF, O_WRONLY | O_CREAT | O_APPEND, 00666);
    if(logger.output_fd == -1) {
        fprintf(stderr, "Cannot open logfile."
                        "Truly bad things happened! aborting\n");
        exit(-1);
    } else {
        hello_msg(argv);
        return;
    }

    return;
}

void lilog_finish(char** argv) {

    if(logger.ring == NULL)
        return;

    char buff[MAX_BUFF] = {0};
    snprintf(buff, MAX_BUFF,  "Process #%d, named %s finished logging", getpid(), argv[0]);
    lilog(INFO, buff);

    int index = get_index();
    append_log(index, FORCE_WRITE);

    fsync(logger.output_fd);
    close(logger.output_fd);
    free(logger.ring);
}
