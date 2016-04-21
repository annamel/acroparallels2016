#include "../../include/mapped_file.h"
#include <stdio.h>

int main(int argc, const char* argv[])
{
	if (argc != 2)
		return -1;

	char buf[20] = "1234";
	mf_handle_t mf = mf_open("file.txt", 0);
	mf_write(mf, 3, 4, buf);
	mf_read(mf, 3, 4, buf);
	printf("%s\n", buf);
	mf_close(mf);

	return 0;
}
