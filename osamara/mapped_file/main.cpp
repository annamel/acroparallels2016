#include "mapped_file.h"

#include <stdio.h>
int main(){
	//printf("1erwerwer\n");
	mf_handle_t mf = mf_open("test");
	//printf("2\n");
	char buf[20	] = "1111";
	mf_write(mf, buf, 4, 3);
	//printf("3\n");
	mf_read(mf, buf, 10, 1);
	//printf("4\n");
	printf("%s\n %d\n", buf, (int)mf_file_size(mf));
	//printf("%d\n", 40*1024*(1024*1024/sysconf(_SC_PAGE_SIZE)));
	
	mf_close(mf);
	//printf("5\n");
	return 0;
}
