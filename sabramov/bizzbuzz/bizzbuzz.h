#ifndef BIZZBUZZ_H
#define BIZZBUZZ_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define BIZZBUZZ_LEN 8

#define KEYBOARD_INPUT(input, out_buf) \
	do { \
		char* str = input; \
		out_buf = parse_string(str); \
	} while(0)

#define FILE_INPUT(out_buf) \
	do { \
		FILE* fp; \
		char* str; \
		fp = fopen("input", "r+"); \
		if (fp == NULL) \
		{ \
			printf("Can't open file! \n"); \
			return -1; \
		} \
		size_t len = 0; \
		getline(&str, &len, fp); \
		out_buf = parse_string(str); \
		fprintf(fp, "%s", out_buf); \
		free(str); \
		fclose(fp); \
	} while(0)


typedef enum deviders
{
	DIV_3 = 1,
	DIV_5 = 2,
	DIV_3_AND_5 = 3
} deviders_t;


inline int pass_char_to_sum(char c);
int check_sum(unsigned long long number, int lenght, int last_char);
void fill_out_buf(int is_div, char **str);
void init_params(int* lenght, char source, unsigned long long* sum, int* last_char);
void deinit_params(int* is_begin, unsigned long long* sum, int* lenght);
void init_pointers(char** source, char** begin, char** end);
char* parse_string(char* input_str);


#endif // BIZZBUZZ_H
