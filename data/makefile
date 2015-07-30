CC=g++
OPTIONS=-std=c++11 -Wfatal-errors -O3
LINK_OPTIONS=-ltbb
all: data.o main.o
	$(CC) $(OPTIONS) $(LINK_OPTIONS) -g main.o data.o -o data
main.o: 
	g++ $(OPTIONS) -c main.cpp -o main.o
data.o:
	g++ $(OPTIONS) -c data.cpp -o data.o
clean:
	rm -f *o data
