

#define lilog_cond(CONDITION)			                        \
    do {                                                                \
        if(CONDITION)                                                   \
            break;                                                      \
        lilog(WARNING, "Condition falied: " #CONDITION);                \
    } while(0)



