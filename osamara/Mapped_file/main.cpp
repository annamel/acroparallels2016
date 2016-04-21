#include "mapped_file_final_api.h"

#include <stdio.h>
int main(){
	mf_handle_t mf = mf_open("test", 0);
	char buf[20] = "1111";
	mf_write(mf, 3, 4, buf);
	mf_read(mf, 1, 10, buf);
	printf("%s\n %d", buf, (int)mf_file_size(mf));
	mf_close(mf);
	return 0;
}
