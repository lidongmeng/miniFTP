main: main.o str.o tunable.o parseconf.o
	g++ -o main main.o str.o tunable.o parseconf.o
str.o: str.cpp
	g++ -c str.cpp
tunable.o: tunable.cpp
	g++ -c tunable.cpp
parseconf.o: parseconf.cpp
	g++ -c parseconf.cpp
main.o: main.cpp
	g++ -c main.cpp
clean:
	rm main *.o
