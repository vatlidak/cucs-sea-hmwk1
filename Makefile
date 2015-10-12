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

checkpatch:
	scripts/checkpatch.pl --no-tree -f src/*

valgrid: clean all
	/usr/bin/valgrind -v --input-fd=3 < ./tests/test.txt ./$(EXECUTABLE)

demo: clean all
	cat ./tests/test.txt | ./main 2>/dev/null

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
