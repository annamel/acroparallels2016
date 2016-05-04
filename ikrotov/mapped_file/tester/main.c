#include <stdio.h>
#include "../chunk_manager/chunk_manager.h"
#include "../mapped_file/mapped_file.h"

int main(void)
{
    int error = 0;

        printf("Start testing\n");

        mf_handle_t mf = mf_open("test.txt");
        if(mf == MF_OPEN_FAILED)
            printf("can't open file\n");

        mf_mapmem_handle_t *mapmem = (mf_mapmem_handle_t *)calloc(1, sizeof(mf_mapmem_handle_t));
        void *ptr = mf_map(mf, 0, 10000, mapmem);
        if(!ptr)
            printf("can't map memory\n");
        else
        {
            for(int i = 0; i < 12289; i++)
                printf("%c ", ((char *)ptr)[i]);

            printf("\n");
        }

        /*error = mf_unmap(mf, *mapmem);
        if(error)
            printf("can't unmap memory: error=%d\n", error);*/


        /*else  //  This attempt is failed
        {
            printf("unmapping success\ntrying to get unmapped memory\n");
            int i = 0;
            while(((char *)ptr)[i] != EOF)
                printf("%c ", ((char *)ptr)[i++]);
            printf("\n");
        }*/

        printf("mf_read tests\n");

        void *buf = (void *)calloc(100, sizeof(char));
        int readed_bytes = mf_read(mf, buf, 5, 0);
        for(int i = 0; i < readed_bytes; i++)
            printf("%c ", ((char *)buf)[i]);
        printf("\n");

        readed_bytes = mf_read(mf, buf, 50, 0);
        for(int i = 0; i < readed_bytes; i++)
            printf("%c ", ((char *)buf)[i]);
        printf("\n");

        readed_bytes = mf_read(mf, buf, 7, 100);
        for(int i = 0; i < readed_bytes; i++)
            printf("%c ", ((char *)buf)[i]);
        printf("\n");

        readed_bytes = mf_read(mf, buf, 100, 10000);
        for(int i = 0; i < readed_bytes; i++)
            printf("%c ", ((char *)buf)[i]);
        printf("\n");

        printf("Finish testing\n");
    return 0;
}

