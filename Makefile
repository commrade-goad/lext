CC = gcc
CFLAGS = -O2 -march=native -Wall
LIBS = -lm -ldl

GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

# Target binary
texm: main.o s7.o
	$(CC) main.o s7.o -o lext $(LIBS)

# Compiles your logic (Takes milliseconds)
main.o: main.c
	$(CC) $(CFLAGS) -DHASHVER=\"$(GIT_HASH)\" -c main.c -o main.o

# Compiles the Lisp engine (Only runs ONCE, or if s7.c changes)
s7.o: s7/s7.c s7/s7.h
	$(CC) $(CFLAGS) -c s7/s7.c -o s7.o

clean:
	rm -f *.o texm lext
