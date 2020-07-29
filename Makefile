

.PHONY: all clean

all: multi-lookup

multi-lookup: multi-lookup.o util.o
	gcc -Wall -Wextra -pthread $^ -o $@


multi-lookup.o : multi-lookup.c multi-lookup.h
	gcc -c -g -Wall -Wextra $<

util.o: util.c util.h
	gcc -c -g -Wall -Wextra $<



clean:
	rm -f multi-lookup
	rm -f *.o $(objects) 
