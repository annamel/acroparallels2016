//{==================================================================================================
//! @file "bizzbuzz.c"
//! @date 2016-02-22
//! @mainpage Program to make special string replacement
//! @author Kirill Shcherbatov
//! @version 1.1
//}==================================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#define handle_error(fmt, args...) \
    do {printf (fmt, ## args); exit(EXIT_FAILURE); } while (0)

#ifdef DEBUG
#define DOUT(fmt, args...) fprintf (stderr, fmt, ## args);
#else
#define DOUT(fmt, args...)
#endif

const int Max_w_buff_size = 4096;

void Usage (char *prg_name);
bool PrintBizzBuzz (uint32_t numword_last_dgt, uint32_t numword_dgt_sum);
void ProcessSource (FILE *chr_source);


int main (int argc, char *argv[]) {
    bool ext_file_mode = (argc >= 3 && !strcmp(argv[1], "-f"));

    if (!ext_file_mode) {
        if (argv[1] && !strcmp(argv[1], "--help")) {
            Usage(argv[0]);
            exit(EXIT_SUCCESS);
        }

        puts("Enter your symbol sequence:");
        ProcessSource (stdin);
    } else {
        FILE *input_file = fopen(argv[2], "r");
        if (!input_file)
            handle_error("Can't open file \"%s\"", argv[2]);

        ProcessSource (input_file);

        fclose(input_file);
    }

    exit(EXIT_SUCCESS);
}


void Usage (char *prg_name) {
    assert (prg_name);
    printf (" Usage: \"%s <flag> <argument>\"\n\n"
            "\t -f - to process some file; send FILENAME as an argument\n"
            "\t\t e.g. %s -f INPUT.TXT\n\n"
            "\t --help to show help\n"
            "\n"
            " This program makes some string replacements:\n"
            "  Each signed integer divisible by 3 replaced with bizz; by 5 - buzz;\n"
            "  by 15 - bizzbuzz; the other characters keeps.\n"
            " Each number should have length less then |%u| to be processed (compile-time const).\n"
            " EOF reaching is a condition to stop reading; Use Ctrl+^D on Linux and Ctrl+^Z on Windows in console mode.\n"
            "", prg_name, prg_name, prg_name, Max_w_buff_size);
}


bool PrintBizzBuzz (uint32_t numword_last_dgt, uint32_t numword_dgt_sum_mod3) {
    DOUT (" # called PrintBizzBuzz(%u, %u)\n", numword_last_dgt, numword_dgt_sum_mod3);
    assert (numword_last_dgt < 10);
    assert (numword_dgt_sum_mod3 < 3);

    bool is_num_div_by_5 = (numword_last_dgt == 0 || numword_last_dgt == 5);
    bool is_num_div_by_3 = (numword_dgt_sum_mod3 == 0);

    if (is_num_div_by_5 && is_num_div_by_3)
        printf("bizzbuzz");
    else if (is_num_div_by_5)
        printf("buzz");
    else if (is_num_div_by_3)
        printf("bizz");
    else
	return false;

    DOUT (" # replacement made\n");

    return true;
}

void ProcessSource (FILE *chr_source) {
    DOUT (" # called ProcessSource( [%p] )\n", chr_source);
    assert (chr_source);

    char chr = 0;
    char w_buff[Max_w_buff_size + 1];
    int32_t w_buff_c_len = 0;

    if ((chr = getc (chr_source)) == EOF) return;

    do {
        /* ----- begin process number block ----- */
        if (chr == '-' || chr == '+') {
            if (w_buff_c_len > 0) // processing smth like --number; first - was pushed at the end of last iteration
                putchar(w_buff[0]);

            DOUT (" # skip sign\n");

            w_buff[0] = chr;
            w_buff_c_len = 1;

            chr = getc (chr_source);
        }

        uint32_t numword_last_dgt = 10;
        uint32_t numword_dgt_sum = 0;

        while (isdigit(chr)) {
            w_buff[w_buff_c_len++] = chr;
            if (w_buff_c_len > Max_w_buff_size)
                handle_error ("\n! Sorry, some number is too big and is not supported yet.\n");

            numword_last_dgt = chr - '0';

            numword_dgt_sum += numword_last_dgt;
            while (numword_dgt_sum >= 3)
                numword_dgt_sum -= 3; //norm num

            chr = getc (chr_source);
        }

        w_buff[w_buff_c_len++] = '\0';
        if (w_buff_c_len > Max_w_buff_size + 1)
            handle_error ("\n ! Sorry, some number is too big and is not supported yet.\n");

        bool rep_made = (numword_last_dgt < 10) ? PrintBizzBuzz (numword_last_dgt, numword_dgt_sum) : false;

        if (!rep_made)
            fputs (w_buff, stdout);
        /* ----- end process number block; !isdigit(chr) now ----- */

        w_buff_c_len = 0;

        if (chr == '+' || chr == '-')
            w_buff[w_buff_c_len++] = chr;
        else putchar(chr);

        DOUT("\n");

    } while ((chr = getc (chr_source)) != EOF);
}
