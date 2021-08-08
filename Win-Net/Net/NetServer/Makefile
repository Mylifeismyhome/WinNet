# build NetServer
export ROOT_DIR=$(PWD)

CFLAGS=-Wall -w

# link Library
define LINK
    ar rvs NetServer.a bin/*.o
    ranlib NetServer.a
endef

# copy Files
define COPY
    mkdir -p ../../bin/
	mkdir -p ../../bin/include/NetServer/
	cp Server.h ../../bin/include/NetServer/
    cp NetServer.a ../../bin/
endef

# clean build
define CLEAN
	rm -r bin/
endef

all:
	mkdir -p bin

	g++ $(CFLAGS) -I ../ -I ../../../extern/ -c Server.cpp -o bin/Server.o

	$(call LINK)
	$(call COPY)
	$(call CLEAN)