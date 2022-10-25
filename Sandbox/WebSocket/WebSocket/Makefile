# build NetWebServer Sandbox
CFLAGS=-std=c++11 -Wall -w -D BUILD_LINUX
INCLUDE_PATH=-I ../../../Win-Net/bin/include/
LIBRARY_PATH=-L ../../../Win-Net/bin/ -l:NetWebSocket.a -l:NetCore.a
LINKFLAGS=-pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) $(INCLUDE_PATH) -c main.cpp -o main.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c WebSocket/WebSocket.cpp -o websocket.o
	g++ $(CFLAGS) -o bin/WebSocket main.o websocket.o $(LIBRARY_PATH) $(LINKFLAGS)

	# copy output binary file
	mkdir -p ../../bin
	cp bin/WebSocket ../../bin
