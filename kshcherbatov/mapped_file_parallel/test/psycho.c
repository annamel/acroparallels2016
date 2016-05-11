#include "../../../include/mapped_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/sysinfo.h>

const char Filename[] = "psycho";
const size_t Iteration_number = 1000000;

int main() {
    //srand( time(0) );

    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        return 0;
    }
    size_t file_size_true = info.totalram*16;
    printf("file_size = %llu", file_size_true);

	int fd = open(Filename, O_RDWR | O_CREAT, 0777);
	ftruncate(fd, file_size_true);
	close(fd);

	mf_handle_t file = mf_open(Filename);
    size_t file_size = mf_file_size(file);

    if (file_size != file_size_true)
        return 555;

    size_t ok_num = 0;

	for (size_t i = 0; i < Iteration_number; i++) {
		mf_mapmem_handle_t loc_handle;
        size_t req_size = rand() % file_size;
        size_t req_offset = rand() % file_size;

		void *loc_ptr = mf_map(file, req_offset, req_size, &loc_handle);
		if (loc_ptr != NULL) {
            ok_num++;
			mf_unmap(file, loc_handle);
		}
	}

    if (ok_num != Iteration_number)
        return 666;

	mf_close(file);

	remove(Filename);
	return 0;
}
