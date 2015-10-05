CC := gcc
CFLAGS := -Wall -Iinclude
LDFLAGS :=
ifeq ($(DEBUG),1)
CFLAGS += -D_DEBUG
endif

OBJECTS := main.o parser.o f_ops.o
EXECUTABLE := main

all: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: src/%.c
	@$(CC) $(CFLAGS) -c $^

check:
	scripts/checkpatch.pl --no-tree -f src/f_ops.c

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
