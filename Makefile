CC := gcc
CFLAGS := -std=c11 -g -static
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

r7cc: $(OBJECTS)
	$(CC) -o r7cc $(OBJECTS) $(LDFLAGS)

clean:
	rm -f r7cc *.o tmp*

format:
	clang-format -i *.h *.c
	git diff --color --exit-code

test: r7cc
	./test.sh

$(OBJECTS): $(wildcard *.h)

.PHONY: clean format test
