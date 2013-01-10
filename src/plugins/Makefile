
CC=$(PREFIX)gcc
CFLAGS=-Wall -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-lc -ldl
INCLUDE=-I../include

all: shell.so audio.so

plugin.o: ../plugin.c
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c ../plugin.c

shell.so: shell.c plugin.o
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c shell.c
	$(CC) -shared -Wl,-soname,shell.so -o shell.so shell.o plugin.o

audio.so: audio.c plugin.o
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c audio.c
	$(CC) -shared -Wl,-soname,shell.so -o audio.so audio.o plugin.o -lm -lasound

clean:
	rm -f *.o *.so
