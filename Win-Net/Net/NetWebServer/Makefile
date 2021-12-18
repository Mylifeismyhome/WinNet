# build NetWebServer
CFLAGS=-std=c++11 -Wall -w -D BUILD_LINUX
INCLUDE_PATH=-I ../ -I ../../../extern/ -I ../../../extern/BCrypt -I ../../../extern/crypto++/bin/include/ -I ../../../extern/OpenSSL/bin/bin/include

# link Library
define LINK
	ar rvs NetWebServer.a bin/*.o
	ranlib NetWebServer.a
endef

# copy Files
define COPY
    	mkdir -p ../../bin/
	mkdir -p ../../bin/include/NetWebServer/
	cp WebSocket.h ../../bin/include/NetWebServer/
    	cp NetWebServer.a ../../bin/
endef

# clean build
define CLEAN
	rm -r bin/
endef

all:
	mkdir -p bin

	g++ $(CFLAGS) $(INCLUDE_PATH) -c WebSocket.cpp -o bin/WebSocket.o

	$(call LINK)
	$(call COPY)
	$(call CLEAN)
