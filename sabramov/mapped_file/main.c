#include "mapped_file.h"
#include "hash_table.h"


int main()
{

	mf_handle_t file = mf_open("test", 200);

	int size = mf_file_size(file);
		
	char* in_buf = "test";
	
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

	mf_read(file, 50, 5, buf1);
	mf_mapmem_t* mem = mf_map(file, 50, 3);
	mf_unmap(mem);

	mf_mapmem_t* mem2 = mf_map(file, 1, 5);
	mf_mapmem_t* mem3 = mf_map(file, 1, 6);	
	
	mf_unmap(mem2);
	mf_unmap(mem3);

	mf_read(file, 100, 5, buf1);

	mf_read(file, 150, 1, buf1);

	mf_read(file, 4100, 15, buf2);

	mf_read(file, 170, 3, buf3);

	mf_read(file, 4100, 17, buf3);

	mf_read(file, 170, 5, buf1);

	mf_read(file, 4100, 20, buf2);

	mf_read(file, 170, 8, buf3);

	mf_read(file, 4100, 23, buf3);

	mf_read(file, 2, 5, buf1);

	mf_read(file, 4152, 3, buf2);

	mf_read(file, 10, 5, buf3);
	
	mf_read(file, 4151, 9, buf3);
	
	mf_read(file, 100, 5, buf3);
	
	mf_read(file, 4150, 5, buf2);

	mf_read(file, 2, 5, buf1);

	mf_read(file, 4152, 3, buf2);

	mf_write(file, 10, 4, in_buf);
	
	mf_read(file, 4151, 9, buf3);
	
	mf_write(file, 100, 4, in_buf);
	
	mf_write(file, 4150, 10, in_buf);

	mf_read(file, 100, 5, buf3);

	mf_write(file, 50, 5, in_buf);

	mf_read(file, 8999, 5, buf3);

	mf_read(file, 12999, 5, buf3);

	mf_read(file, 17000, 5, buf3);		

	
	mf_close(file);

	return 0;
}