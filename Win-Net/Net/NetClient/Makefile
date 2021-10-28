# build NetClient
CFLAGS=-std=c++11 -Wall -w -D BUILD_LINUX
INCLUDE_PATH=-I ../ -I ../../../extern/ -I ../../../extern/BCrypt -I ../../../extern/crypto++/bin/include/ -I ../../../extern/OpenSSL/bin/bin/include

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

	g++ $(CFLAGS) $(INCLUDE_PATH) -c Client.cpp -o bin/Client.o

	$(call LINK)
	$(call COPY)
	$(call CLEAN)
