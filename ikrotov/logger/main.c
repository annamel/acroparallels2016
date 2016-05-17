#include <stdio.h>
#include "logger.h"

int main(void)
{
    logger_t* logger = log_init();
    log_set_default_loglvl(DEBUG);
    log_write(INFO, "Some very usefull information about program");
    log_write(DEBUG, "Some program debug information");
    log_write(ERROR, "Some information about error in program");
    log_deinit();
    return 0;
}

