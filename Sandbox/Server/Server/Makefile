# build NetServer Sandbox
export ROOT_DIR=$(PWD)

CFLAGS=-std=c++11 -Wall -w

all:
	mkdir -p bin

	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c main.cpp -L ../../../Win-Net/bin/ -l:NetCore.a -l:NetServer.a -o main.o
	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c Server/Server.cpp -L ../../../Win-Net/bin/ -l:NetCore.a -l:NetServer.a -o server.o
	g++ $(CFLAGS) -o bin/Server -L ../../../Win-Net/bin/ -l:NetCore.a -l:NetServer.a server.o main.o -pthread -ldl -lresolv
