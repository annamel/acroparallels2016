/* bizzbuzz.c
 *
 * Copyright (C) 2016  <aglab2@mail.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <err.h>

#define EXTSTRING_DEFSIZE 32
//String that is able to change its size dynamically
struct extstring{
	char * data;
  	int size;
  	int cursize;
};

enum readstate{
	READSTATE_START,
	READSTATE_ISNUMBER,
	READSTATE_NOTNUMBER
};

int extstring_init(struct extstring * es);
int extstring_add_char(struct extstring * es, char c);
int extstring_clear(struct extstring * es);
void extstring_dispstr(struct extstring * es);
int extstring_destroy(struct extstring * es);


//Convert int to Zk (% k op)
int int2Z(int n, int k);

void usage(char * pname){
	printf("Usage: %s OPTION [FILE]\n", pname);
  	printf("bizzbuzz is simple program that from user input (file or console)\n");
  	printf("changes all numbers that are divisible by 3 to 'bizz',\n");
  	printf("divisible by 5 to 'bazz' and divisible by 3 and 5 to 'bizzbass'.\n");
  	printf("If number is not divisible by 3 or 5 it remains unchanged.\n");
  	printf("All numbers can be sepated by any kind of whitespaces, all \n");
  	printf("non-numbers or whitespaces remains unchanged\n\n");
  	printf("Operation modes:\n");
  	printf("  %s help       	# shows this help\n", pname);
  	printf("  %s console	# reads input from console\n", pname);
  	printf("  %s file FILE	# reads input from file FILE\n", pname);
}

int main(int argc, char * argv[]){
  	FILE * input;

  	//Parse input arguments
  	if (argc < 2){
		usage(argv[0]);
	  	return 1;
	}else if (argc == 2){
		if (!strcmp(argv[1], "help")){
		  	usage(argv[0]);
			return 0;
		}else if (!strcmp(argv[1], "console")){
		  	input = stdin;
		}else{
			usage(argv[0]);
	  		return 1;
		}
	}else if (argc == 3){
		if (!strcmp(argv[1], "file")){
			input = fopen(argv[2], "rb");
		  	if (input == NULL){
				err(1, "%s", argv[2]);
			}
		}else{
			usage(argv[0]);
	  		return 1;
		}
	}

	char cur = 0;
  	char prev = 0;
	struct extstring * es = (struct extstring *) malloc(sizeof(struct extstring)); //String for read number
  	extstring_init(es);
  	int sum3 = 0; //Sum of numbers for div3
  	enum readstate state = READSTATE_START;



  	cur = fgetc (input);
	do{
		if (isspace(cur)){ //Print out all the stored data
		  	if (state == READSTATE_ISNUMBER){
				//As it was int we should display it
				if (sum3 == 0)
					printf("bizz");
				if (prev == '0' || prev == '5')
					printf("bazz");
				if (sum3 != 0 && prev != '0' && prev != '5')
				  	extstring_dispstr(es);
			}
			else if (state == READSTATE_NOTNUMBER){
				//Print the bad string with red color
				if (input == stdin){ //Actually this comparison is ok
					printf("\033[31m");
		  			extstring_dispstr(es);
					printf("\033[0m");
				}else{
				  	extstring_dispstr(es);
				}
			}

			if (state != READSTATE_START) { //Setup for new inputs
							//
			  	extstring_clear(es);
			  	sum3 = 0;
			}

			state = READSTATE_START;
			putchar (cur);
		}else{
		  	extstring_add_char(es, cur);

			//Make num from char
			int num = cur - '0';

		  	if (num < 0 || num > 9){
			 	if ((cur == '-' || cur == '+') && state == READSTATE_START) { //Allow unary ops
					state = READSTATE_ISNUMBER;
				}else{
			  		state = READSTATE_NOTNUMBER;
					fprintf(stderr, "Your input contains wrong symbol (%c)!\n", cur);
				}
			}else{
			  	if (state == READSTATE_START || state == READSTATE_ISNUMBER){
			  		state = READSTATE_ISNUMBER;
			  		sum3 += num;
			  		sum3 = int2Z(sum3, 3);
				}
			}
		}
		prev = cur;
	  	cur = fgetc (input);
	}while (!feof(input));



	//Flush the last symbols
	if (state == READSTATE_ISNUMBER){
		//As it was int we should display it
		if (sum3 == 0)
			printf("bizz");
		if (prev == '0' || prev == '5')
			printf("bazz");
		if (sum3 != 0 && prev != '0' && prev != '5')
			extstring_dispstr(es);
	}else if (state == READSTATE_NOTNUMBER){
		//Print the bad string with red color
		if (input == stdin){
			printf("\033[31m"); //Set red color - probably should be removed tho
		  	extstring_dispstr(es);
			printf("\033[0m"); //Set default color
		}else{
			extstring_dispstr(es);
		}
	}

	extstring_destroy(es);
  	if (input != stdin) fclose(input);
	free(es);
	return 0;
}




int extstring_init(struct extstring * es){
  	if (!es) return -1;

	es -> data = (char *) calloc(EXTSTRING_DEFSIZE, 1);
  	es -> size = EXTSTRING_DEFSIZE;
  	es -> cursize = 0;
  	return 0;
}

int extstring_add_char(struct extstring * es, char c){
  	if (!es) return -1;
  	if (!es -> data) return -2;

  	es -> cursize++;
  	if (es -> cursize - 1 >= es -> size){
		while (es -> cursize >= es -> size)
	  		es -> size *= 2;
	  	es -> data = (char *) realloc(es -> data, es -> size);
	}

	es -> data[es -> cursize-1] = c;
  	return 0;
}

int extstring_clear(struct extstring * es){
  	if (!es) return -1;
  	if (!es -> data) return -2;

  	es -> cursize = 0;
  	memset(es -> data, 0, es -> size);
}

void extstring_dispstr(struct extstring * es){
  	if (!es) return;
  	if (!es -> data) return;
	fwrite(es -> data, 1, es -> cursize, stdout);
}

int extstring_destroy(struct extstring * es){
	if (!es) return -1;
  	if (!es -> data) return -2;
  	free(es -> data);
  	return 0;
}

int int2Z(int n, int k){
  	if (k < 0) return -1;
	while (n > 0){
	  	n -= k;
	}
	while (n < 0){
	  	n += k;
	}
	return n;
}
