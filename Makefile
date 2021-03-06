CC := gcc
CFLAGS := -Wall -Iinclude
LDFLAGS :=
ifeq ($(DEBUG),1)
CFLAGS += -D_DEBUG
endif

OBJECTS := main.o parser.o f_ops.o
EXECUTABLE := ./main
TEST := ./tests/test.txt

build: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXECUTABLE) $(OBJECTS)

%.o: src/%.c
	@$(CC) $(CFLAGS) -c $^

test: clean build
	cat $(TEST) | $(EXECUTABLE) 2>/dev/null

exec: clean build
	$(EXECUTABLE) 2>/dev/null

valgrid: clean build
	/usr/bin/valgrind -v --input-fd=3 < $(TEST) $(EXECUTABLE)

checkpatch:
	scripts/checkpatch.pl --no-tree -f src/*

clean:
	rm -f $(EXECUTABLE)
	rm -f $(OBJECTS)
