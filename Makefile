CC := gcc
CFLAGS := -Wall -Iinclude
LDFLAGS :=
ifeq ($(DEBUG),1)
CFLAGS += -D_DEBUG
endif

OBJECTS := main.o parser.o fops.o
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
