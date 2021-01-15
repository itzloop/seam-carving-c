CC = gcc
# -Wall -O3
CFLAGS =  -Wall -pedantic -lm -g lib/gifenc/gifenc.c
OBJECTS = main.o

default: $(OBJECTS)

clean:
	rm *.o

%.o: %.c
	$(CC) -o$@ $< $(CFLAGS)

.PHONY: clean