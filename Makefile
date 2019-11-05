CC := gcc
CFLAGS := -std=c11 -g -static
SOURCES := $(wildcard *.c)
OBJECTS := $(SOURCES:.c=.o)

cc7: $(OBJECTS)
	$(CC) -o cc7 $(OBJECTS) $(LDFLAGS)

clean:
	rm -f cc7 *.o tmp*

format:
	clang-format -i *.h *.c

test: cc7
	./test.sh

$(OBJECTS): $(wildcard *.h)

.PHONY: clean format test
