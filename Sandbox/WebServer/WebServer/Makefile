# build NetWebServer Sandbox
export ROOT_DIR=$(PWD)

CFLAGS=-Wall -w
LINKFLAGS=-pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c main.cpp -o main.o
	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c WebServer/WebServer.cpp -o webserver.o
	g++ $(CFLAGS) -o bin/WebServer main.o webserver.o -L ../../../Win-Net/bin/ -l:NetWebServer.a -l:NetCore.a $(LINKFLAGS)