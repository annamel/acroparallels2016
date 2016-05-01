#include <mapped_file.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
int main(){
	printf("EHEHEHHEHEH\n");
	fflush(stdout);
	remove("osamara.txt");
	int fd = open("osamara.txt", O_CREAT|O_RDWR, 0666);
	const char buff[] = "111111111111111111111111111111111111111111";
	printf("%d | %d\n", fd, (int )write(fd, buff, strlen(buff)));
	fflush(stdout);
	printf("%d\n",close(fd));
	fflush(stdout);
	printf("1erwerwer\n");
	fflush(stdout);
	mf_handle_t mf = mf_open("osamara.txt");
	printf("2\n");
	char buf[20] = "1111";
	printf("23\n");
	mf_write(mf, buf, 4, 3);
	printf("3\n");
	mf_read(mf, buf, 10, 1);
	printf("4\n");
	printf("%s\n %d\n", buf, (int)mf_file_size(mf));
	//printf("%d\n", 40*1024*(1024*1024/sysconf(_SC_PAGE_SIZE)));
	
	mf_close(mf);
	printf("5\n");
	return 0;
}
