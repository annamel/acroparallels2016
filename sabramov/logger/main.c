#include "logger.h"

int main()
{

	log_init(INFO_LEVEL, LOG_FILE, 2);
	
	log_message(INFO_LEVEL, "func1 return 0x0");
	log_message(INFO_LEVEL, "Running func2");
	log_message(WARNING_LEVEL, "Func 3 exited with th code 0x%x", 7);
	log_message(WARNING_LEVEL, "Address: 0x%x, offset: 0x%x, number of bytes: %d", 0x80000, 0x8, 24);
	
	log_message(FATAL_LEVEL, "FATAL: Func returns negetive value!"); 
	
	return 0;
}


