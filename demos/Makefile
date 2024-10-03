SRCS = \
	0_getopt_tplt.c \
	1_getopt_long_tplt.c \
	2_getopt_longer_tplt.c \
	getopt_longer.o

OBJS = $(SRCS:.c=.o)

TARGETS = \
	0_getopt_tplt \
	1_getopt_long_tplt \
	2_getopt_longer_tplt

CFLAGS = -g -Wall -Wstrict-prototypes

default: $(TARGETS)

clean:
	rm -f $(OBJS) *~

immaculate: clean
	rm -f $(TARGETS)

0_getopt_tplt: 0_getopt_tplt.o
	$(CC) $(CFLAGS) $^ -o $@

1_getopt_long_tplt: 1_getopt_long_tplt.o
	$(CC) $(CFLAGS) $^ -o $@

$(OBJS): getopt_longer.h

2_getopt_longer_tplt: 2_getopt_longer_tplt.o getopt_longer.o
	$(CC) $(CFLAGS) $^ -o $@
