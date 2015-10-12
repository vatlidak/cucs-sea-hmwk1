CC := gcc
CFLAGS := -Wall -Iinclude
LDFLAGS :=
ifeq ($(DEBUG),1)
CFLAGS += -D_DEBUG
endif

OBJECTS := main.o parser.o f_ops.o
EXECUTABLE := ./main
TESTS := ./tests/*

build: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: src/%.c
	@$(CC) $(CFLAGS) -c $^

test: clean build
	cat $(TESTS) | $(EXECUTABLE) 2>/dev/null

exec: clean build
	$(EXECUTABLE) 2>/dev/null

valgrid: clean build
	/usr/bin/valgrind -v --input-fd=3 < $(TESTS) $(EXECUTABLE)

checkpatch:
	scripts/checkpatch.pl --no-tree -f src/*

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
