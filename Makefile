CC = gcc
# -Wall -O3
CFLAGS =  -std=c17 -Wall -pedantic -lm -g lib/gifenc/gifenc.c
OBJECTS = main.o

default: $(OBJECTS)

clean:
	rm *.o

%.o: %.c
	$(CC) -o$@ $< $(CFLAGS)

.PHONY: clean