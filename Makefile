CC = gcc
CFLAGS = -O2 -march=native -Wall
LIBS = -lm -ldl -rdynamic

GIT_HASH := $(shell git rev-parse --short HEAD 2>/dev/null || echo "unknown")

FFI_CFLAGS := $(shell pkg-config --cflags libffi 2>/dev/null || echo "-I/usr/lib64/libffi/include")
FFI_LIBS   := $(shell pkg-config --libs   libffi 2>/dev/null || echo "-lffi")

# All lext object files
LEXT_OBJS = main.o lext_types.o lext_io.o lext_ffi.o lext_module.o \
            lext_capture.o lext_da.o lext_builtins.o

# Target binary
lext: $(LEXT_OBJS) s7.o nob.o
	$(CC) $(LEXT_OBJS) s7.o nob.o -o lext $(LIBS) $(FFI_LIBS)

# lext modules (share the same flags)
LEXT_CFLAGS = $(CFLAGS) $(FFI_CFLAGS) -DHASHVER=\"$(GIT_HASH)\"

main.o: src/main.c src/lext_hash.h src/lext_types.h src/lext_io.h src/lext_ffi.h src/lext_module.h \
        src/lext_capture.h src/lext_da.h src/lext_builtins.h
	$(CC) $(LEXT_CFLAGS) -c src/main.c -o main.o

lext_types.o: src/lext_types.c src/lext_types.h src/lext_hash.h
	$(CC) $(LEXT_CFLAGS) -c src/lext_types.c -o lext_types.o

lext_io.o: src/lext_io.c src/lext_io.h src/lext_types.h
	$(CC) $(LEXT_CFLAGS) -c src/lext_io.c -o lext_io.o

lext_ffi.o: src/lext_ffi.c src/lext_ffi.h src/lext_types.h src/lext_io.h src/lext_hash.h
	$(CC) $(LEXT_CFLAGS) -c src/lext_ffi.c -o lext_ffi.o

lext_module.o: src/lext_module.c src/lext_module.h src/lext_hash.h
	$(CC) $(LEXT_CFLAGS) -c src/lext_module.c -o lext_module.o

lext_capture.o: src/lext_capture.c src/lext_capture.h src/s7/s7.h
	$(CC) $(LEXT_CFLAGS) -c src/lext_capture.c -o lext_capture.o

lext_da.o: src/lext_da.c src/lext_da.h src/s7/s7.h
	$(CC) $(LEXT_CFLAGS) -c src/lext_da.c -o lext_da.o

lext_builtins.o: src/lext_builtins.c src/lext_builtins.h src/s7/s7.h
	$(CC) $(LEXT_CFLAGS) -c src/lext_builtins.c -o lext_builtins.o

# Compiles the Lisp engine (Only runs ONCE, or if s7.c changes)
s7.o: src/s7/s7.c src/s7/s7.h
	$(CC) $(CFLAGS) -c src/s7/s7.c -o s7.o

nob.o: src/nob.c src/nob.h
	$(CC) $(CFLAGS) -c src/nob.c -o nob.o

clean:
	rm -f *.o lext
