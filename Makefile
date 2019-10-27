CFLAGS=-std=c11 -g -static

cc7: cc7.c

clean:
	rm -f cc7 tmp tmp.s

compile:
	gcc -o cc7 cc7.c

test: cc7
	./test.sh

.PHONY: test clean
