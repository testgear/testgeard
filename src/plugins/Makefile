CC=$(PREFIX)gcc
CFLAGS=-Wall -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS=-lc -ldl
INCLUDE=-I../include

all: dummy.so fb.so shell.so audio.so

plugin.o: ../plugin.c
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c ../plugin.c

dummy.so: dummy.c plugin.o
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c dummy.c
	$(CC) -shared -Wl,-soname,dummy.so -o dummy.so dummy.o plugin.o

fb.so: fb.c plugin.o
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c fb.c
	$(CC) -shared -Wl,-soname,fb.so -o fb.so fb.o plugin.o

shell.so: shell.c plugin.o
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c shell.c
	$(CC) -shared -Wl,-soname,shell.so -o shell.so shell.o plugin.o

audio.so: audio.c plugin.o
	$(CC) $(CFLAGS) $(INCLUDE) -fPIC -c audio.c
	$(CC) -shared -Wl,-soname,shell.so -o audio.so audio.o plugin.o -lm -lasound -laudiofile

clean:
	rm -f *.o *.so

.PHONY: all clean
