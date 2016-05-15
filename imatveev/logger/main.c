#include <stdio.h>
#include "logger.h"

int main () {
        #ifdef DEBUG
        printf("DEBUG\n");
        #endif
        set_log_level(warning);
        set_log_file("my_file");
        LOG(debug,   "GAME OVER_1");
        LOG(info,    "GAME OVER_2");
        LOG(warning, "GAME OVER_3");
        LOG(error,   "GAME OVER_4");
        LOG(fatal,   "GAME OVER_5");
        LOG(warning, "GAME OVER_6");
        return 0;
}
