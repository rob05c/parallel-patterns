CC=g++
OPTIONS=-std=c++11
LINK_OPTIONS=-ltbb
all: collectives.o main.o
	$(CC) $(OPTIONS) -g main.o collectives.o -o collectives $(LINK_OPTIONS)
main.o: 
	g++ $(OPTIONS) -c main.cpp -o main.o
collectives.o:
	g++ $(OPTIONS) -c collectives.cpp -o collectives.o
clean:
	rm -f *o collectives
