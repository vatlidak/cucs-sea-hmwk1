CC := gcc
CFLAGS := -Wall -Iinclude
LDFLAGS :=


OBJECTS := main.o parser.o
EXECUTABLE := main

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: src/%.c
	$(CC) $(CFLAGS) -c $^

check:
	scripts/checkpatch.pl --no-tree -f src/*

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
.PHONY: clean
