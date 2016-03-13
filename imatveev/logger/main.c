#include <stdio.h>
#include "logger.h"

int main () {
        #ifdef DEBUG
        printf("DEBUG\n");
        #endif
        log_init();
        log_error(TRUE, ERROR, "GAME OVER");
        log_end();
        return 0;
}
