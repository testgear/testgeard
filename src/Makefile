#
# testgeard - Test Gear device server
#

VERSION=0.2alpha

CC=$(PREFIX)gcc
CFLAGS=-Wall -DVERSION=\"$(VERSION)\" -DSERVER -Iinclude -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
LDFLAGS=-lc -ldl

all: testgeard plugins

debug: CFLAGS += -DDEBUG -g
debug: all

objects = main.o \
          plugin-manager.o \
          connection-manager.o \
          command-manager.o \
          tcp.o \
          options.o \
          daemon.o \
          message.o \
          list.o

$(objects): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

testgeard: $(objects)
	$(CC) $(LDFLAGS) $(objects) -o $@ -ldl

plugins:
	make -C plugins

clean:
	rm -f *.o testgeard
	make -C plugins clean

.PHONY : all clean debug plugins
