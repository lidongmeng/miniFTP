main: main.o str.o tunable.o parseconf.o sysutil.o session.o ftpproto.o privparent.o
	g++ -g -o main main.o str.o tunable.o parseconf.o sysutil.o session.o ftpproto.o privparent.o -lcrypt
ftpproto.o: ftpproto.cpp
	g++ -g -c ftpproto.cpp
privparent.o: privparent.cpp
	g++ -g -c privparent.cpp
str.o: str.cpp
	g++ -g -c  str.cpp
session.o: session.cpp
	g++ -g -c session.cpp
sysutil.o: sysutil.cpp
	g++ -g -c  sysutil.cpp
tunable.o: tunable.cpp
	g++ -g -c  tunable.cpp
parseconf.o: parseconf.cpp
	g++ -g -c  parseconf.cpp
main.o: main.cpp
	g++ -g -c  main.cpp
clean:
	rm main *.o
