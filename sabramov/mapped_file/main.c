#include "mapped_file.h"
#include "hash_table.h"


int main()
{

	mf_handle_t file = mf_open("test", 200);

	int size = mf_file_size(file);
		
	char in_buf[5] = "test";

	char buf1[10];
	char buf2[10];
	char buf3[10];
	
	memset(buf1, '\0', 10);
	memset(buf2, '\0', 10);	
	memset(buf3, '\0', 10);

	mf_read(file, 50, 5, buf1);	
	mf_mapmem_t* mem0 = mf_map(file, 50, 5);
	mf_mapmem_t* mem1 = mf_map(file, 4150, 5);	

	char* str = "mfile";
	memcpy(mem1->ptr, str, 5);
	mf_read(file, 4150, 5, buf1);
//	printf("buf 1: %s\n", buf1);
	
	mf_write(file, 50, 3, in_buf);
	mf_read(file, 50, 3, buf2);
//	printf("buf 2: %s\n", buf2);

	mf_unmap(mem0);	
	mf_unmap(mem1);

	mf_write(file, 4150, 4, in_buf);
	mf_read(file, 4150, 4, buf3);

//	printf("buf 3: %s\n", buf3);

	
	mf_close(file);

	return 0;
}