#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int words_count = 0;

int getNumOfWords() {
    return words_count;
}

void reset(){
    words_count = 0;
}

int getNumberOfWords(char* path)
{
    if(!path)
        return -1;

    int fd = open(path, O_RDONLY);
    if(fd == -1)
        return -1;

    int len = 0;

    len = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    char* str = (char*)calloc(1, len);
    if(!str)
        return -1;

    if(read(fd, str, len) == -1)
        return -1;

    int w_count = 0;
    char* token = 0;

    token = strtok(str, " ");

    do {

        w_count++;
        for(int i = 0; i < strlen(token) + 1; i++)
        {

            if(     ((token[i] >= 'a') && (token[i] <= 'z'))
                ||  ((token[i] >= 'A') && (token[i] <= 'Z'))
                ||  (token[i] == '-') ) { continue;
            } else {

                if(token[i] == '\n' || token[i] == '\0')
                    break;

                w_count--;

                break;
            }

        }

    } while (token = strtok(NULL, " "));

    words_count += w_count;
    return words_count;
}

/*
int main() {
    int a = 0;

    a = getNumberOfWords("test");

    printf("%d \n", a);

    return 0;
}
*/
