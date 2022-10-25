# build NetClient Sandbox
CFLAGS=-std=c++11 -Wall -w -D BUILD_LINUX
INCLUDE_PATH=-I ../../../Win-Net/bin/include/
LIBRARY_PATH=-L ../../../Win-Net/bin/ -l:NetClient.a -l:NetCore.a
LINKFLAGS=-pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) $(INCLUDE_PATH) -c main.cpp -o main.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c Client/Client.cpp -o client.o
	g++ $(CFLAGS) -o bin/Client main.o client.o $(LIBRARY_PATH) $(LINKFLAGS)

	# copy output binary file
	mkdir -p ../../bin
	cp bin/Client ../../bin/
