# build NetWebServer Sandbox
CFLAGS=-std=c++11 -Wall -w
INCLUDE_PATH=-I ../../../Win-Net/bin/include/ -I ../../../extern/
LIBRARY_PATH=-L ../../../Win-Net/bin/ -l:NetWebServer.a -l:NetCore.a
LINKFLAGS=-pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) $(INCLUDE_PATH) -c main.cpp -o main.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c WebServer/WebServer.cpp -o webserver.o
	g++ $(CFLAGS) -o bin/WebServer main.o webserver.o $(LIBRARY_PATH) $(LINKFLAGS)
