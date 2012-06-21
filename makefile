

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
	./png2c --input=rgb.png --output=out.png --c_name=out --color_depth=RGB565 