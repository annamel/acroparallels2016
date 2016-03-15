#include <string.h>

#include "interfaces.h"

void get_demangled_function_name(char* function_name, const char* input)
{
	strncpy(function_name, input, FUNCTION_NAME_SIZE);
}
