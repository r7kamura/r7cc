compile:
	gcc -o cc7 cc7.c

clean:
	rm -f cc7

test: cc7
	./cc7

.PHONY: clean compile test
