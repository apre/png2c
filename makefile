

CC = gcc
CFLAGS= -O2 -c
LIBS= -lpng

OBJS=png2c.o

all: png2c test


.c.o:
	$(CC) $(CFLAGS) $<

png2c: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

test:
	./png2c tests/tux.png dtux.png