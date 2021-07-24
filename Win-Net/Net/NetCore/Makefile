## build NetCore
export ROOT_DIR=$(PWD)

# Net/Net/
define PATH_NET
	g++ -I ../ -I ../../../../extern/OpenSSL/include -w -c ../Net/Net/Net.cpp
	g++ -I ../ -c -w ../Net/Net/NetCodes.cpp
	g++ -I ../ -c -w ../Net/Net/NetVersion.cpp
	g++ -I ../ -c -w ../Net/Net/NetString.cpp
endef

# link Library
define LINK
	ar rvs NetCore.a Net.o NetCodes.o NetVersion.o NetString.o
endef

# copy Files
define COPY
	mkdir -p ../../bin/
	cp NetCore.a ../../bin/

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
	g++ -I ../ -w -c ../Net/Cryption/PointerCryption.h
	$(call LINK)
	$(call COPY)
	$(call CLEAN)
