main: main.o processor.o
	g++ bin/main.o bin/processor.o -o main

main.o: main.cpp
	g++ main.cpp -c -o bin/main.o

processor.o: processor.cpp
	g++ processor.cpp -c -o bin/processor.o

clean:
	rm bin/main.o main bin/processor.o
