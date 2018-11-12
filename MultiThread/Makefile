main: main.o Nqueens.o NqEventLoop.o CalThread.o
	g++ -std=c++11 -o $@ $^ -lpthread
main.o: main.cpp Nqueens.h NqEventLoop.h
	g++ -std=c++11 -c main.cpp
Nqueens.o: Nqueens.cpp Nqueens.h
	g++ -std=c++11 -c  Nqueens.cpp
NqEventLoop.o: NqEventLoop.cpp NqEventLoop.h CalThread.h
	g++ -std=c++11 -c  NqEventLoop.cpp
CalThread.o: CalThread.cpp CalThread.h Nqueens.h
	g++ -std=c++11 -c  CalThread.cpp
all: main