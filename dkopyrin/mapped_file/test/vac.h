#ifndef __VAC_H_
#define __VAC_H_

#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

#define VAC(command) do{							\
	struct rlimit rl;							\
	if (getrlimit(RLIMIT_AS, &rl)){						\
		printf("Fetch error, %s", strerror(errno));			\
	}									\
	struct rlimit rl_good = {.rlim_cur=128*GB, .rlim_max=rl.rlim_max};	\
	struct rlimit rl_bad = {.rlim_cur=128*MB, .rlim_max=rl.rlim_max};	\
	if (setrlimit(RLIMIT_AS, &rl_bad))					\
		printf("Fail_bad, %s\n", strerror(errno));			\
	command;								\
	if (setrlimit(RLIMIT_AS, &rl_good))					\
		printf("Fail_good, %s\n", strerror(errno));			\
}while(0)

#endif
