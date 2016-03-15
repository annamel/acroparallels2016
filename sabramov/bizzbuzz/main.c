/**
  * This bizzbuzz code get 1 string as input 										
  * and replaces all integer numbers that divided by 
  *	N without residue with str :  
  * N = 3 => str = "bizz",
  * N = 5 => str = "buzz",
  * if N can equals both 3 and 5 => str = "bizzbuzz". 
  * Characters that are not numbers stay without 
  * any manipulations on them. Numbers that don't divided 
  *	by 3 or 5 without residue also stay without action, 
  * Code suuports two input modes: from the keyboard and from
  * file called "input". If you want to enter string from the
  * keyboard, please enter string from linux command line IN QUOTES
  * (example: ./bizzbuzz "etrt0 +++ __ --60" ). But you should remmember
  * that quotes are PROHIBITED inside your input string.
  * If you want to get string from the "input" file, please, 
  * just write your string in "input" file and 
  *	run bizzbuzz program without any parameters. Output string
  * will appear in same "input" file as second line.
  * @uthor: Semyon Abramov 
  * semyon.abramov.mipt@gmail.com 
  **/   

#include "bizzbuzz.h"

int main(int argc, char* argv[])
{
	char* out_buf;
	
	if (argc == 1)
		FILE_INPUT(out_buf);
	else
	{
		if (argc == 2)
		{
			KEYBOARD_INPUT(argv[1], out_buf);
			printf("output string: %s \n", out_buf);
		}
		else
		{
			printf("Incorrect input !!! \n");
			printf("Please, enter 1 string in quotes\n" 
				   "or just run program to get string from the input file\n");
			return 1;
		}	 
					
	}
 
	free (out_buf);
	return 0;
	 
}		

