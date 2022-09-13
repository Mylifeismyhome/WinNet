# build Feature Sandbox to test functionality
CFLAGS=-std=c++11 -Wall -w -D BUILD_LINUX
INCLUDE_PATH=-I ../../../Win-Net/bin/include/
LIBRARY_PATH=-L ../../../Win-Net/bin/ -l:NetCore.a
LINKFLAGS=-pthread -ldl -lresolv

all:
	mkdir -p bin

	g++ $(CFLAGS) $(INCLUDE_PATH) -c main.cpp -o main.o
	g++ $(CFLAGS) -o bin/Feature main.o $(LIBRARY_PATH) $(LINKFLAGS)

	# copy output binary file
	mkdir -p ../../bin
	cp bin/Feature ../../bin/
