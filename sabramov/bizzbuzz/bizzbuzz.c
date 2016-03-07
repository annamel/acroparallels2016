#include "bizzbuzz.h"
			

inline int pass_char_to_sum(char c)
{
	return (c - '0');
}  	

int sum_of_chars(char* line)
{
	int sum = 0, i = 0;
	int len = strlen(line);

	while (i < len)
	{
		sum += (line[i] - '0');
		i++;
	}

	return sum;
} 

int check_sum(unsigned long long number, int lenght, int last_char)
{
	long long int sum = number, len = 0;
	char* line = malloc(sizeof(char) * lenght);
	int is_div3 = 0, is_div5 = 0;
	
	memset(line, 0, sizeof(char) * lenght);
	
	while (!(sum >= 0 && sum <= 9))
	{
		len = sprintf(line, "%lld", sum);
		sum = sum_of_chars(line);
	}
	
	if (sum == 3 || sum == 6 || 
		sum == 9 || sum == 0)
		is_div3 = 1;

	if (last_char == 0 || last_char == 5)
		is_div5 = 1;

	
	free(line);
		
	return (is_div5 << 1) | is_div3;
}
	 
void fill_out_buf(int is_div, char **str)
{
	switch (is_div)
	{
		case DIV_3:
			strcat(*str, "BIZZ");
			break;
		case DIV_5:
			strcat(*str, "BUZZ");
			break;
		case DIV_3_AND_5:
			strcat(*str, "BIZZBUZZ");
			break;
	}	
}				

void init_params(int* lenght, char source, unsigned long long* sum, int* last_char)
{
	*lenght ++;
	*sum += pass_char_to_sum(source);
	*last_char = pass_char_to_sum(source);
}

void deinit_params(int* is_begin, unsigned long long* sum, int* lenght)
{
	*is_begin = 0;
	*sum = 0;
	*lenght = 0;
}

void init_pointers(char** source, char** begin, char** end)
{
   	*end = *source;
	*source = *begin;
	*begin = NULL;
}

char* parse_string(char* input_str) 
{
	char* begin_str; 
	char* end_str;
	char* out_buf = malloc(sizeof(char) * strlen(input_str) * BIZZBUZZ_LEN);
	int is_begin = 0, is_div = 0, lenght = 0, last_char = 0;
	unsigned long long sum = 0;
	
	while (input_str[0] != '\0')
	{
		if (!(input_str[0] >= '0' && input_str[0] <= '9'))
		{
			if (is_begin)
	 		{
	 			if (is_div = check_sum(sum, lenght, last_char))
				{
					fill_out_buf(is_div, &out_buf);	 				
	 				deinit_params(&is_begin, &sum, &lenght);
	 			}
	 			else
	 			{
					init_pointers(&input_str, &begin_str, &end_str);
					deinit_params(&is_begin, &sum, &lenght);					
					
					while (input_str != end_str)
					{
			 			out_buf[strlen(out_buf)] = input_str[0];
			 			input_str++;
					}				 
						
				}	 			
	 		}
	 		else
	 		{
				out_buf[strlen(out_buf)] = input_str[0];
	 			input_str++;
	 		}
	 	}
		else
	 	{
			if (!is_begin)
	 		{
	 			begin_str = input_str;
	 			is_begin = 1;
	 		}

	 		init_params(&lenght, input_str[0], &sum, &last_char);
	 			 
	 		if (input_str[1] == '\0') 
			{
				if (is_div = check_sum(sum, lenght, last_char))
				{
					fill_out_buf(is_div, &out_buf);	 				
	 				deinit_params(&is_begin, &sum, &lenght);
				}
				else
				{
					init_pointers(&input_str, &begin_str, &end_str);

					while (input_str != end_str )
					{
			 			out_buf[strlen(out_buf)] = input_str[0];
			 			input_str++;
					}				
				 		
			 		out_buf[strlen(out_buf)] = input_str[0];
				}	
			}
				
			input_str++;
		}	
	}
	
	return out_buf;
}

	 			
