#include "mf.h"
#include "../logger/log.h"

int main(int argc, char *argv[]){
	struct mf * file = malloc(sizeof(struct mf));
	LOG(DEBUG, "---Open\n");
	mf_open(argv[0], file);
  	void * buffy = malloc(12345);
	LOG(DEBUG, "---Read 1\n");
	mf_read(file, buffy, 12345);
	LOG(DEBUG, "---Read 2\n");
	mf_read(file, buffy, 12345);
	LOG(DEBUG, "---Close\n");

	mf_close(file);
	free(buffy);
  	free(file);
}
