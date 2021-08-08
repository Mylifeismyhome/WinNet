# build NetClient
export ROOT_DIR=$(PWD)

CFLAGS=-Wall -w

# link Library
define LINK
        ar rvs NetClient.a bin/*.o
        ranlib NetClient.a
endef

# copy Files
define COPY
        mkdir -p ../../bin/
	mkdir -p ../../bin/include/NetClient/
	cp Client.h ../../bin/include/NetClient/
        cp NetClient.a ../../bin/
endef

# clean build
define CLEAN
        rm -r bin/
endef

all:
	mkdir -p bin

	g++ $(CFLAGS) -I ../ -I ../../../extern/ -c Client.cpp -o bin/Client.o

	$(call LINK)
	$(call COPY)
	$(call CLEAN)
