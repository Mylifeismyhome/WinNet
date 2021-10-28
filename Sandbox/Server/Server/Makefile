# build NetServer Sandbox
CFLAGS=-std=c++11 -Wall -w -D BUILD_LINUX
INCLUDE_PATH=-I ../../../Win-Net/bin/include/ -I ../../../extern/
LIBRARY_PATH=-L ../../../Win-Net/bin/ -l:NetServer.a -l:NetCore.a
LINKFLAGS=-pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) $(INCLUDE_PATH) -c main.cpp -o main.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c Server/Server.cpp -o server.o
	g++ $(CFLAGS) -o bin/Server main.o server.o $(LIBRARY_PATH) $(LINKFLAGS)
