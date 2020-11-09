
test: main.o sub.o
	gcc -Wall -o test main.o sub.o

%.o: %.c
	gcc -Wall -c -o $@ $<

sub.o: sub.h

clean:
	rm *.o test -f
