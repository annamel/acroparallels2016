#ifdef __cplusplus
	#include <cxxabi.h>
#endif
#include <stdlib.h>
#include <string.h>

#include "interfaces.h"

void get_demangled_function_name(char* function_name, const char* input)
{
	if (input[0] == '_')
	{
		size_t length = FUNCTION_NAME_SIZE;
		int status;
		char* demangled = abi::__cxa_demangle(input, NULL, &length, &status);
		if (demangled)
		{
			snprintf(function_name, FUNCTION_NAME_SIZE, "%s[%s]", demangled, input);
			free(demangled);
		}
		else
			strncpy(function_name, input, FUNCTION_NAME_SIZE);
	}
	else
		strncpy(function_name, input, FUNCTION_NAME_SIZE);
}
