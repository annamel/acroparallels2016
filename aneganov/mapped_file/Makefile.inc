CFLAGS = -Wall --std=gnu11 -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter
CFLAGS += $(if $(or $(DEBUG), $(LOG), $(DEBUG2)), -O0, -Ofast)
CFLAGS += $(if $(or $(DEBUG), $(DEBUG2)), -g -DDEBUG,)
CFLAGS += $(if $(DEBUG2), -DDEBUG2,)
CFLAGS += $(if $(LOG), -DLOG_ON -fprofile-arcs -ftest-coverage,)
