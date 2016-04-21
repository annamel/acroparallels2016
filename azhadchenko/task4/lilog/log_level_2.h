#define lilog_cond_msg(CONDITION, MSG, ...)                     \
    do {                                                        \
        if(CONDITION)                                           \
            break;                                              \
                                                                \
        lilog(ERROR, "Condition failed: "                       \
            #CONDITION " with msg: " MSG, __VA_ARGS__);         \
                                                                \
    } while(0)

