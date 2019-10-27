CC := gcc
CFLAGS := -std=c11 -g -static

cc7: cc7.c

clean:
	rm -f cc7 tmp tmp.s

test: cc7
	./test.sh

.PHONY: test clean
