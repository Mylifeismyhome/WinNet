# build NetServer Sandbox
export ROOT_DIR=$(PWD)

CFLAGS=-std=c++11 -Wall -w -pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c main.cpp -o main.o
	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c Server/Server.cpp -o server.o
	g++ $(CFLAGS) -o bin/Server main.o server.o -L ../../../Win-Net/bin/ -l:NetServer.a -l:NetCore.a
