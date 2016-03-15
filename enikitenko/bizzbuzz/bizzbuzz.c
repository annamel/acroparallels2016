#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define INITIAL_STRING_SIZE 1024

#define BIZZ "bizz"
#define BUZZ "buzz"
#define DELIMETER " \n\t"

bool is_divided_by_3(const char* str);
bool is_divided_by_5(const char* str);
void print_number(const char* str);
bool is_number(const char* str);

void run_bizzbuzz(char* input);
int run_file_bizzbuzz(const char* filename);
int run_command_line_bizzbuzz();
void print_usage(const char* name);

bool is_divided_by_3(const char* str)
{
	int i = 0;
	if (str[i] == '-')
		i = 1;

	int sum = 0;	
	for (; str[i]; i++)
		sum += str[i] - '0';

	while (sum > 0)
		sum -= 3;

	return sum == 0;
}

bool is_divided_by_5(const char* str)
{
	int len = strlen(str);
	return str[len - 1] == '0' || str[len - 1] == '5';
}

void print_number(const char* str)
{
	bool divided_by_3 = is_divided_by_3(str);
	bool divided_by_5 = is_divided_by_5(str);

	if (divided_by_3 && divided_by_5)
		printf(BIZZ BUZZ " ");
	else if (divided_by_3)
		printf(BIZZ " ");
	else if (divided_by_5)
		printf(BUZZ " ");
	else
		printf("%s ", str);
}

bool is_number(const char* str)
{
	int i = 0;

	if (str[0] == '-')
		i = 1;
	for (; str[i]; i++)
	{
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

void run_bizzbuzz(char* input)
{
	char* str = strtok(input, DELIMETER);
	while (str)
	{
		int n;
		if (is_number(str))
			print_number(str);
		else
			printf("%s ", str);

		str = strtok(NULL, DELIMETER);
	}
	printf("\n");
}

int run_file_bizzbuzz(const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (!file)
	{
		fprintf(stderr, "Error: can't open file \"%s\"\n", filename);
		return 255;
	}
	
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	char* data = malloc(length);
	if (!data)
	{
		fprintf(stderr, "Error: malloc failed\n");
		fclose(file);
		return 255;
	}

	fseek(file, 0, SEEK_SET);
	fread(data, 1, length, file);

	run_bizzbuzz(data);

	free(data);
	fclose(file);

	return 0;
}

int run_command_line_bizzbuzz()
{
	char* buffer = malloc(INITIAL_STRING_SIZE);
	int i;
	int size = INITIAL_STRING_SIZE;
	if (!buffer)
	{
		fprintf(stderr, "Error: malloc failed\n");
		return 255;
	}

	int c;
	while ((c = fgetc(stdin)) != EOF)
	{
		if (i >= INITIAL_STRING_SIZE)
		{
			size *= 2;
			buffer = realloc(buffer, size);
			if (!buffer)
			{
				fprintf(stderr, "Error: malloc failed\n");
				return 255;
			}
		}

		buffer[i] = (char) c;
		i++;

		if (c == '\n')
		{
			buffer[i - 1] = 0;
			run_bizzbuzz(buffer);
			i = 0;
		}
	}
	
	free(buffer);

	return 0;
}

void print_usage(const char* name)
{
	printf("\nOpenBizzBuzz\n");
	printf("Usage:\n");
	printf("%s                 -- run command line bizzbuzz (using stdin)\n", name);
	printf("%s --file filename -- run bizzbuzz using file\n", name);
	printf("%s --help          -- show this message\n", name);
	printf("BizzBuzz divides input string into words and replaces all numbers that are divided by 3 and by " BIZZ BUZZ ",\n");
	printf("all numbers that are divided by 3 by " BIZZ " and all numbers that are divided by 5 by " BUZZ ".\n");
	printf("Other words won't replaced.\n");
	printf("Numbers must be decimal, without point and unary plus.\n");
	printf("Correct numbers: 0, -7, 2718281828459045, -3\n");
	printf("Incorrect numbers: 5.0, 2a, --4, +7, a2, 2.\n");
	printf("Incorrect numbers (like other text) won't replaced\n\n");
}

int main(int argc, char* argv[])
{
	if (argc == 3 && !strcmp(argv[1], "--file"))
		return run_file_bizzbuzz(argv[2]);
	else if (argc == 1)
		return run_command_line_bizzbuzz();
	else
	{
		bool help_cmd = (argc == 2 && !strcmp(argv[1], "--help"));
		if (!help_cmd)
			fprintf(stderr, "Error: incorrect arguments\n");

		print_usage(argv[0]);
		return help_cmd ? 0 : 255;
	}
}
