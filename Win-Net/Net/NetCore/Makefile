## build NetCore
export ROOT_DIR=$(PWD)

# Net/Net/
define PATH_NET
	g++ -w -I ../ -I ../../../../extern/OpenSSL/include -c ../Net/Net/Net.cpp
	g++ -w -I ../ -c ../Net/Net/NetCodes.cpp
	g++ -w -I ../ -c ../Net/Net/NetVersion.cpp
	g++ -w -I ../ -c ../Net/Net/NetString.cpp
endef

# Net/Cryption/
define PATH_CRYPTION
	g++ -w -I ../ -c ../Net/Cryption/XOR.cpp
endef

# link Library
define LINK
	ar rvs NetCore.a Net.o NetCodes.o NetVersion.o NetString.o XOR.o
endef

# copy Files
define COPY
	mkdir -p ../../bin/
	cp NetCore.a ../../bin/

	rm -r ../../bin/include/
	mkdir -p ../../bin/include/
	cp -r ../Net/**/*.h --parents ../../bin/include/
	mv ../../bin/Net ../../bin/include
endef

# clean build
define CLEAN
	rm *.o
	find ../../ -type f -name '*.h.gch' -delete
endef

all:
	$(call PATH_NET)
	$(call PATH_CRYPTION)
	g++ -I ../ -w -c ../Net/Cryption/PointerCryption.h
	$(call LINK)
	$(call COPY)
	$(call CLEAN)
