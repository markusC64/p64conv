.PHONY : all clean
all: p64conv

clean:
	rm -f p64conv.o p64.o p64conv

p64conv: p64conv.o p64.o
	g++ -O3 -o p64conv p64conv.o p64.o -lre2 -lpthread
	
p64conv.o: p64conv.cpp
	g++ -O3 -Ilib/p64refimp -c p64conv.cpp


p64.o: lib/p64refimp/p64.c
	gcc -O3 -Ilib/p64refimp -c lib/p64refimp/p64.c
