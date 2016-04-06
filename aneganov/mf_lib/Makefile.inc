CFLAGS = -Wall --std=gnu11 -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter
CFLAGS += $(if $(DEBUG),-g -DDEBUG -O0, -Ofast)
CFLAGS += $(if $(DEBUG2), -DDEBUG2,)
CFLAGS += $(if $(LOG), -DLOG_ON,)
