# build NetWebServer
export ROOT_DIR=$(PWD)

CFLAGS=-Wall -w

# link Library
define LINK
    ar rvs NetWebServer.a bin/*.o
    ranlib NetWebServer.a
endef

# copy Files
define COPY
    mkdir -p ../../bin/
	mkdir -p ../../bin/include/NetWebServer/
	cp WebServer.h ../../bin/include/NetWebServer/
    cp NetWebServer.a ../../bin/
endef

# clean build
define CLEAN
	rm -r bin/
endef

all:
	mkdir -p bin

	g++ $(CFLAGS) -I ../ -I ../../../extern/ -I ../../../extern/OpenSSL/bin/bin/include -c WebServer.cpp -o bin/WebServer.o

	$(call LINK)
	$(call COPY)
	$(call CLEAN)