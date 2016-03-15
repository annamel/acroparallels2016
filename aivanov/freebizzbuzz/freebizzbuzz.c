#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ERROR(msg)						\
do										\
{										\
	fprintf(stderr, "Error: %s\n", msg);\
	exit(1);							\
										\
} while (1)

typedef struct
{
	char* data;
	unsigned size;
	unsigned max_size;
	
} mutable_buffer_t;

int buffer_init(mutable_buffer_t* buffer)
{
	buffer->max_size = 1;
	buffer->size = 0;
	buffer->data = malloc(buffer->max_size);
	
	return buffer->data == NULL;
}

void buffer_print(mutable_buffer_t* buffer)
{
	unsigned i;
	for (i = 0; i < buffer->size; i++)
		putc(buffer->data[i], stdout); // @jjeka: можно было бы занулить buffer->data[buffer->size] и воспользоваться puts, это должно немного увеличить производительность.
		//@eviom Но прийдется вечно лишний байт хранить, что может привести к лишнему выделению. Так что не факт.
}

void buffer_reset(mutable_buffer_t* buffer)
{
	buffer->size = 0;
}

void buffer_free(mutable_buffer_t* buffer)
{
	free(buffer->data);
	
	buffer->size = 0;
	buffer->max_size = 0;
	buffer->data = NULL;
}

int buffer_push(mutable_buffer_t* buffer, char c)
{
	if (buffer->size + 1 > buffer->max_size)
	{
		buffer->max_size *= 2;
		buffer->data = realloc(buffer->data, buffer->max_size);
		
		if (!buffer->data)
			return 1;
	}
	
	buffer->data[buffer->size++] = c;
	
	return 0;
}

unsigned sum_mod_3(unsigned a, unsigned b)
{
	assert(a < 3 && b < 3);
	
	unsigned c = a + b;
	
	return (c < 3) ? c : c - 3;
}

unsigned digit_mod_3(unsigned a)
{
	assert(a <= 9);
	
	if (a < 3)
		return a;
	else if (a < 6)
		return a - 3;
	else if (a < 9)
		return a - 6;
	else
		return 0;
}

void print_usage(const char* executable)
{
	printf("\n");
	printf("Usage: %s [--input <input file>]\n\n", executable);
	printf("Replaces only whole numbers from input\n"
		   "\tto bizzbuzz if 3 and 5 are its delimiters\n"
		   "\tto bizz if 3 is\n"
		   "\tto buzz if 5 is\n"
		   "Where\n"
		   "\twhole number is a string of format [\\+\\-]*[0-9]+\n"
		   "\tfraction is a string of format [\\+\\-]*[0-9]*\\.[0-9]*\n"
		   "\tthe input string (with them excluded) format is [^0-9\\+\\-\\.]\n"
		   "\tmoreover, no sign character precedes a cut number\n"
		   "\n"
		   "The resulting string is then printed to stdout.\n"
		   "\n"
		   "--input <input file>: specify input file as source, otherwise stdin is used.\n"
		   "\n");
}

void bizzbuzz(FILE* input) // @jjeka Большая функция, было бы неплохо ее разбить
							//@eviom Да, но проблематично из-за кучи переменных. Конечно, можно запихнуть их в структурку...
{	
	unsigned mod_3 = 0;	
	int last_digit = 0;
	bool reading_number = false;
	bool reading_signs = false;
	bool reading_fraction = false;
	
	mutable_buffer_t delayed_output;
	if (buffer_init(&delayed_output))
		ERROR("buffer_init: Insufficient memory");

	while (true) // @jjeka мне кажется, что использовать for (;;) более логично (хотя тут всегда споры)
				//@eviom Да начнется холивар!
	{
		int c = fgetc(input);
		
		if (isdigit(c))
		{
			if (!reading_number)
			{
				if (!reading_signs)
					buffer_reset(&delayed_output);
					
				mod_3 = 0;
				reading_number = true;
				reading_signs = false;
				last_digit = 0;
			}
			
			last_digit = c - '0';
			mod_3 = sum_mod_3(mod_3, digit_mod_3((unsigned) last_digit));
			
			if (buffer_push(&delayed_output, c))
				ERROR("buffer_push: Insufficient memory");
		}
		else 
		{			
			if (reading_number)
			{
				int printed = 0;
				
				if (c != '.' && !reading_fraction)
				{
					if (!mod_3)
					{
						printf("bizz");
						printed = 1;
					}
			
					if (last_digit == 0 || last_digit == 5)
					{
						printf("buzz");
						printed = 1;
					}
				}
			
				if (!printed)
				{
					buffer_print(&delayed_output);
				}
				
				reading_number = false;
				reading_fraction = false;
			}
			
			reading_fraction = (c == '.');
			
			if (c == '+' || c == '-')
			{
				if (!reading_signs)
				{
					buffer_reset(&delayed_output);
					reading_signs = true;
				}
				
				if (buffer_push(&delayed_output, c))
					ERROR("buffer_push: Insufficient memory");
			}
			else
			{
				if (reading_signs)
				{
					buffer_print(&delayed_output);
					reading_signs = false;
				}
				
				if (c != EOF)
					putc(c, stdout);
			}
		}
		
		if (c == EOF)
			break;
	}
	
	buffer_free(&delayed_output);
}

int main(int argc, char** argv)
{
	FILE* input = NULL;
	
	if (argc == 3 && !strcmp(argv[1], "--input"))
	{
		input = fopen(argv[2], "rt");
		
		if (!input)
			ERROR("Unable to open input file");
	}
	else if (argc == 2 && !strcmp(argv[1], "--help"))
	{
		print_usage(argv[0]);
		return 0;
	}
	else if (argc != 1)
		ERROR("Invalid parametes. Launch with --help");
		
	bizzbuzz(input ? input : stdin);
	
	if (input)
		fclose(input);
		
	return 0;
}
