CFLAGS = -Wall -std=gnu11 -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter
CFLAGS += $(if $(or $(DEBUG), $(PROF), $(DEBUG2), $(ASAN)), -O0, -Ofast)
CFLAGS += $(if $(or $(DEBUG), $(DEBUG2), $(ASAN)), -g -DDEBUG,)
CFLAGS += $(if $(DEBUG2), -DDEBUG2,)
CFLAGS += $(if $(LOG), -DLOG_ON,)
CFLAGS += $(if $(PROF), -fprofile-arcs -ftest-coverage,)
CFLAGS += $(if $(ASAN), -fsanitize=address -fno-omit-frame-pointer,)
LDFLAGS += $(if $(ASAN), -lasan,)

ifeq ($(CC),icc)
	LDFLAGS += -lirc
endif
