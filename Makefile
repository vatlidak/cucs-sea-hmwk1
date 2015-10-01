CC := gcc
CFLAGS := -w -Wall
OBJ=$(wildcard *.o)

SRC := main.c
TARGET := main

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) $(OBJ)
