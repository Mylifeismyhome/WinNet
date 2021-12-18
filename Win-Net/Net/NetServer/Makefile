# build NetServer
CFLAGS=-std=c++11 -Wall -w -D BUILD_LINUX
INCLUDE_PATH=-I ../ -I ../../../extern/ -I ../../../extern/BCrypt -I ../../../extern/crypto++/bin/include/ -I ../../../extern/OpenSSL/bin/bin/include

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

	g++ $(CFLAGS) $(INCLUDE_PATH) -c Server.cpp -o bin/Server.o

	$(call LINK)
	$(call COPY)
	$(call CLEAN)
