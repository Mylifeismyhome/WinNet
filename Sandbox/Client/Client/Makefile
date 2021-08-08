# build NetClient Sandbox
export ROOT_DIR=$(PWD)

CFLAGS=-Wall -w
LINKFLAGS=-pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c main.cpp -o main.o
	g++ $(CFLAGS) -I ../../../Win-Net/bin/include/ -I ../../../extern/ -c Client/Client.cpp -o client.o
	g++ $(CFLAGS) -o bin/Client main.o client.o -L ../../../Win-Net/bin/ -l:NetClient.a -l:NetCore.a $(LINKFLAGS)