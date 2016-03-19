#include <stdio.h>
#include "logger.h"

int main () {
        #ifdef DEBUG
        printf("DEBUG\n");
        #endif
        set_log_level(warning);
        set_log_file("my_file");
        log_error(debug,   "GAME OVER_1");
        log_error(info,    "GAME OVER_2");
        log_error(warning, "GAME OVER_3");
        log_error(error,   "GAME OVER_4");
        log_error(fatal,   "GAME OVER_5");
        log_error(warning, "GAME OVER_6");
        return 0;
}
