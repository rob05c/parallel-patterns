CC=g++
OPTIONS=-std=c++11
LINK_OPTIONS=-ltbb

all: forkjoin.o main.o
	$(CC) $(OPTIONS) $(LINK_OPTIONS) -g main.o forkjoin.o -o forkjoin
main.o:
	$(CC) $(OPTIONS) -c main.cpp -o main.o
forkjoin.o:
	$(CC) $(OPTIONS) -c forkjoin.cpp -o forkjoin.o
clean:
	rm -f *o forkjoin
