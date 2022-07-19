CC=gcc
SD=sender
RC=receiver
SRC=$(RC).c $(SD).c
CFLAGS=-g -Wall -W
OBJ=$(SRC:.c=.o)
LIBFILE=lib.a
TOOLS=network.c packet_implem.c window.c

all: $(TOOLS) $(LIBFILE) $(RC) $(SD)

$(SD): $(LIBFILE) $(SD).o
	$(CC) $(CFLAGS) -o ../$(SD) $(SD).o $(LIBFILE) -lz

$(RC): $(LIBFILE) $(RC).o
	$(CC) $(CFLAGS) -o ../$(RC) $(RC).o $(LIBFILE) -lz

$(LIBFILE): $(TOOLS:.c=.o)
	$(AR) r $@ $(TOOLS:.c=.o)

clean:
	rm *.o $(LIBFILE)

.PHONY: clean
