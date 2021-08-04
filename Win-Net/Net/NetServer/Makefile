# build NetServer
export ROOT_DIR=$(PWD)

CFLAGS=-std=c++11 -Wall

# link Library
define LINK
        ar rvs NetServer.a bin/*.o
        ranlib NetServer.a
endef

# copy Files
define COPY
        mkdir -p ../../bin/
        cp NetServer.a ../../bin/
endef

# clean build
define CLEAN
        rm -r bin/
endef

all:
	mkdir -p bin

	g++ $(CLFAGS) -I ../ -I ../../../extern/ -c Server.cpp -L ../../bin/ -l:NetCore.a -o bin/Server.o

	$(call LINK)
	$(call COPY)
	$(call CLEAN)
