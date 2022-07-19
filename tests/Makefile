CC=gcc
WAY=../src/
SRC=test.c $(WAY)packet_implem.c $(WAY)network.c $(WAY)window.c
CFLAGS=-g -Wall -W
OBJ=$(SRC:.c=.o)

all: test clear

test: $(OBJ)
	gcc $(CFLAGS) -o test $(OBJ) -lz

clear:
	rm *.o

clean:
	rm test

.PHONY: clean
