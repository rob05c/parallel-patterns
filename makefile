CC=g++
OPTIONS=-std=c++11
LINK_OPTIONS=-ltbb
all: patterns.o main.o
	$(CC) $(OPTIONS) $(LINK_OPTIONS) -g main.o patterns.o -o patterns
main.o: 
	g++ $(OPTIONS) -c main.cpp -o main.o
patterns.o:
	g++ $(OPTIONS) -c patterns.cpp -o patterns.o
clean:
	rm -f *o patterns
