// на вход программе подается имя файла, из которого будут считаны все строки
// пробелы, табы и интеры считаются разделителями
// строки должны состоять из целых чисел - "слова" не являющиеся целыми числами игнорируются
// числа делящиеся на 3 заменяются на bizz
// числа делящиеся на 5 заменяются на buzz
// числа делящиеся на 3 и на 5 заменяются на bizzbuzz
// и выводятся через пробел в стандартный поток вывода.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int size_buf = 256;
 
void work_with_word(char *str) {
	int flag_3 = 0, flag_5 = 0, flag_num = 1;
	unsigned long long summa = 0; // сумма цифр числа
	// игнорируем знак числа
	char *ptr = (*str == '+' || *str == '-') ? str + 1 : str;
	// проверяем что число и считаем сумму цифр
	while (*ptr != '\0') {
		summa += *ptr - '0';
		if (*ptr < '0' || *ptr > '9')
			flag_num = 0;
		ptr++;
	}
	if (flag_num) {
		// проверка делимости на 5
		char last_ch = str[strlen(str)-1];
		if (str[0] != '\0' && (last_ch == '5' || last_ch == '0'))
			flag_5 = 1;
		// проверка делимости на 3
		char buffer[50]; // long long влезет
		int i;
		do {
			sprintf(buffer, "%lld", summa);
			summa = 0;
			for (i = 0; buffer[i] != '\0'; i++)
				summa += buffer[i] - '0';
		} while (strlen(buffer) > 1);
		last_ch = buffer[0];
		if (last_ch == '0'|| last_ch == '3' || last_ch == '6' || last_ch == '9')
			flag_3 = 1;
		// вывод на экран
		if (flag_3 && flag_5)
			printf("bizzbuzz ");
		else
		if (flag_3)
			printf("bizz ");
		else
		if (flag_5)
			printf("buzz ");
		else
			printf("%s ", str);
	}
}

void work_with_string(char *str) {
	char *ptr = str;
	char *sep = " \t\n";
	ptr = strtok(str, sep);
	do {
		work_with_word(ptr);
	} while ((ptr = strtok(NULL, sep)) != NULL);
}

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("error: print only file name\n");
		exit(-1);
	}
	FILE* file = fopen(argv[1], "r");
	if (file == NULL) {
		printf("error: Can't open file %s\n", argv[1]);
		exit(-1);
	}
	char buffer[size_buf];
	unsigned int size_str = size_buf;
	char *str = calloc(size_str, sizeof(char));
	while (fgets(buffer, size_buf, file) != NULL) {
		//довыделение памяти
		if (strlen(buffer) + strlen(str) >= size_str) {
			size_str *= 2;
			if (realloc(str, size_str) == NULL) {
				printf("error: can't allocate mamory\n");
				exit(-2);
			}
		}
		strcat(str, buffer);
		if (strlen(buffer) < size_buf-1) {
			// обработка строки
			work_with_string(str);
			str[0] = '\0'; // обнуление строки
		}
	}
	printf("\n");
	return 0;
}
