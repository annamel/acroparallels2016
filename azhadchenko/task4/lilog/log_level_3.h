#define BACKTRACE_MAX 16

void lilog_print_stack(char* str) {
    void* buffer[BACKTRACE_MAX] = {0};
    char** strings = 0;
    int left_len = MAX_BUFF;
    size_t offset = 0;

    int nptrs = backtrace(buffer, BACKTRACE_MAX);

    strings = backtrace_symbols(buffer, nptrs);
    if(!strings)
        lilog(DEBUG, "Unable to get backtrace");

    int index = get_index();
    char* tmp = prepare_log(DEBUG, index, &left_len);

    offset += snprintf(tmp, left_len, "%s\n", str);

    for(int i = 0; i < nptrs; i++)
        if(left_len - offset > 0)
            offset += snprintf(tmp + offset, left_len - offset, "%s\n", strings[i]);


    logger.ring[GET_CONTROL_BIT(index)] = WRITTEN_BUFFER;

    append_log(index, AUTOMATIC_WRITE);

    free(strings);
}

#define _if(CONDITION)                                          \
    if(!CONDITION) {                                            \
        if(BACKTRACE_ENABLE)                                    \
            lilog_print_stack("Condition falied: " #CONDITION); \
        else                                                    \
            lilog_cond_inner(CONDITION, DEBUG);                 \
    } else if(CONDITION)


