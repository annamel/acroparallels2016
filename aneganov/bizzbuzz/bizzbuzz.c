/**********************************************************************************************************
 * The "BizzBuzz" program
 * 
 * Author: Alexey Neganov <neganovalexey@gmail.com>
 * 
 * Standard: C99
 * Platform: Linux
 ***********************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

/***********************************************************************************************************
 * Let consider a fact that every power of 4 has the remaider of the division by 3 equals to 1.
 * Every power of 2, which is not a power of 4, has the remaider of the division by 3 equals to 2.
 * Let x be equal to (3a + r1 + 3b + r2). Obviously, the remainder of x equals to remainder of (r1 + r2).
 * 
 * The following algorithm represents the number as series of consecutive bit positions, 
 * gets the sum of the remainders of powers of 2 from this series, and than process it recursively.
 * 
 * Let calcucate the complexity.
 * T(n) = T(n) + O(log(n)) ==> T(n) ~ O(log(n))
 **********************************************************************************************************/

static bool __is_3_divisible(long int val) {
    if(val == 0 || val == 3) /* 0, 1, 2, 3 -- possible sums of two bit positions */
        return true;
    if(val == 1 || val == 2)
        return false;

    long int newval = 0;
    while(val != 0) {
        newval += val & 3; /* +1, if least significant bit posision is not 0, +2, if second position is not 0 */
        val >>= 2;
    }
    return __is_3_divisible(newval);
}

static bool is_3_divisible(long int val) {
	assert(val > LONG_MIN);
    if(val < 0)
        val = -val;
    return __is_3_divisible(val);
}

/********************************************************************************************************
 * The idea of the following function is the same as described above.
 * 
 *    Number | Reminder (div by 5)
 *       1   |    1
 *       2   |    2
 *       4   |    4
 *       8   |    3
 *      16   |    1
 *      32   |    2
 *      64   |    4
 *     128   |    3
 *    ............................
 * 	The rigorious proof is obvious, so I decided not to place it here.
 *******************************************************************************************************/

static bool __is_5_divisible(long int val) {
    if(val == 0 || val == 5)
        return true;
    if(val == 1 || val == 2 || val == 3 || val == 4 || val == 6 || val == 7)
        return false;

    long int newval = 0;
    while(val != 0) {
        newval += val & 7;
        newval += ( (val & 8) != 0)? 3 : 0;
        val >>= 4;
    }
    return __is_5_divisible(newval);
}

static bool is_5_divisible(long int val) {
	assert(val > LONG_MIN);
    if(val < 0)
        val = -val;
    return __is_5_divisible(val);
}

#define handle_format_error(msg, args) \
    do {fprintf(stderr, msg, args); exit(EXIT_FAILURE);} while(0)

#define handle_error(msg) \
    do {perror(msg); exit(EXIT_FAILURE);} while(0)

static bool print_bizzbuzz(char * str) {
    const int numeral_system_base = 10;

    char *endptr = NULL;

    errno = 0;
    long int val = strtol(str, &endptr, numeral_system_base);

    if (errno != 0)
        handle_error("strtol"); /* "Value is out of range", actually */

    if (endptr == str) {
        if(strpbrk(str, "0123456789") != NULL) /* lexems as 5a, a5 and similar are forbidden */
            handle_format_error("format error: %s\n", str);
        printf("%s", str);
        return false;
    }
    else if (*endptr != '\0')
        handle_format_error("format error: %s\n", str);

    bool val_is_3_divisible = is_3_divisible(val);
    bool val_is_5_divisible = is_5_divisible(val);

    if(!val_is_3_divisible && !val_is_5_divisible)
        printf("%ld", val);
    if(val_is_3_divisible)
        printf("bizz");
    if(val_is_5_divisible)
        printf("buzz");

    return true;
}

/*******************************************************************************************************
 * The file I/O is slow, so it may be bufferised. Lexems longer than buffer size are not supported.
 * However, there is a possible situation that some lexem is on the border of the buffer frame.
 * To deal with it there is a mechanism of saving the suspicious fragments -- remainders -- before the
 * overwriting of the buffer.
 ******************************************************************************************************/

#define BUF_SIZE (64 * 1024)
char BUF[BUF_SIZE] = {0};

static void process_a_file(const char * filename) {
    const char * delim = " \f\n\v\r\t";

    int fd = open(filename, O_RDONLY);
    if(fd == -1)
        handle_error("open");

    int k = 0;
    char * reminder = NULL;
    int str_len = 0;
    while( (k = read(fd, BUF, BUF_SIZE - 1)) > 0) {
		memset(BUF + k, 0, BUF_SIZE - k);
		
        char * str = BUF;

        if(reminder != NULL) {			
            if(isspace(BUF[0]) || BUF[0] == 0)
                print_bizzbuzz(reminder);
                
            /* "Splicing" of the remainder and the new fragment */
            else {
                int rem_len = str_len;

                char * endptr = strpbrk(str, delim);
                if(endptr == NULL)
                    endptr = strchr(str, 0);

                str_len = endptr - str;
                *endptr = 0;

                char * aggregate_str = (char *)calloc(str_len + rem_len + 1, sizeof(char));
                strncpy(aggregate_str, reminder, rem_len);
                strncpy(aggregate_str + rem_len, str, str_len);
                print_bizzbuzz(aggregate_str);

                free(aggregate_str);

                str = endptr + 1;
            }
            printf(" ");
            free(reminder);
            reminder = NULL;
        }

        while(str < BUF + BUF_SIZE - 1) {
            char * endptr = strpbrk(str, delim);
            if(endptr == NULL) {
				endptr = strchr(str, 0);
				
				/* Saving the reminder */
				if(endptr == BUF + BUF_SIZE - 1) {
					str_len = endptr - str;
					reminder = (char *)calloc(str_len + 1, sizeof(char));
					strncpy(reminder, str, str_len);
					break;
				}
            }
            *endptr = 0;
            if(str != endptr) {
                print_bizzbuzz(str);
                printf(" ");
            }
            str = endptr + 1;
        }
    }
    if(k == -1)
        handle_error("read");

    if(reminder != NULL) {
        print_bizzbuzz(reminder);
        free(reminder);
    }

    printf("\n");

    if(close(fd) == -1)
        handle_error("close");
}

static void print_usage(const char * const program_name, FILE * const stream, const int exit_code) {
    fprintf(stream, "Usage: %s options [ file ]\n", program_name);
    fprintf(stream,
        "   -h   --help                     Display this usage information\n"
        "   -f   --file          filename   Get input from file\n"
        "   -c   --command-line             Get input from the command line\n"
        "\n"
        "The input format is a set of whole numbers and a words without digits, divided "
        "(and possibly started/terminated) by space symbols.\n"
        "The output format is a set of the same numbers and words, divided "
        " by space symbols, excluding numbers divisible by 3 and 5, "
        "which are replaced with 'bizz', 'buzz' and 'bizzbuzz' accordingly.\n"
        "Lexems longer than 64 KB and numbers out of long int range are not supported.\n");
    exit(exit_code);
}

int main(int argc, char *argv[]) {
    if ( argc < 2 )
        print_usage(argv[0], stderr, EXIT_FAILURE);

    const char * const short_options = "hf:c";

    const struct option long_options[] = {
        {"help",         0, NULL, 'h'},
        {"file",         1, NULL, 'f'},
        {"command-line", 0, NULL, 'c'},
        {NULL,           0, NULL,  0}
    };

    int option = getopt_long(argc, argv, short_options, long_options, NULL);

    switch(option) {
        case 'h':    /* -h or --help */
            print_usage(argv[0], stdout, EXIT_SUCCESS);
        case 'f':    /* -f or --file */
            if( argc < 3 )
                print_usage(argv[0], stderr, EXIT_FAILURE);
            process_a_file(argv[2]);
            break;
        case 'c':    /* -c or --command-line */
            for(int i = 2; i < argc; i++) {
                print_bizzbuzz(argv[i]);
                if(i < argc - 1)
                    printf("%s", " ");
                else printf("\n");
            }
            break;
        default:
            print_usage(argv[0], stderr, EXIT_FAILURE);
            break;
    }
    exit(EXIT_SUCCESS);
}

