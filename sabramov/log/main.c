#include "logger.h"

int main()
{
	log_message(INFO_LEVEL, "Func swap() return 0x0");
	log_message(INFO_LEVEL, "Running func code_gen()");
	log_message(WARNING_LEVEL, "Func code_gen() exited with the code 0x%x", 7);
	log_message(WARNING_LEVEL, "Address: 0x%x, offset: 0x%x, number of bytes: %d", 0x80000, 0x8, 24);
	log_message(INFO_LEVEL, "Function sets errno to EINVAL");
	log_message(INFO_LEVEL, "Func round_up() returns number of bytes: %d", 8);
	log_message(WARNING_LEVEL, "Number of free chunks 0x%x", 8);
	log_message(WARNING_LEVEL, "Address: 0x%x, offset: 0x%x, number of bytes: %d", 0x0da0c715, 0x3, 19);
	
	log_message(FATAL_LEVEL, "Stack overflow");

	return 0;
}


