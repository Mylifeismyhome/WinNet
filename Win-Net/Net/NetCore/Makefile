# build NetCore
export ROOT_DIR=$(PWD)

# Net/Net/
define PATH_NET
	g++ -w -I ../ -I ../../../extern/OpenSSL/include -c ../Net/Net/Net.cpp
	g++ -w -I ../ -c ../Net/Net/NetCodes.cpp
	g++ -w -I ../ -c ../Net/Net/NetVersion.cpp
	g++ -w -I ../ -c ../Net/Net/NetString.cpp
endef

# Net/Cryption/
define PATH_CRYPTION
	g++ -w -I ../ -c ../Net/Cryption/XOR.cpp
	g++ -I ../ -w -c ../Net/Cryption/PointerCryption.h
	g++ -I ../ -I ../../../extern -w -c ../Net/Cryption/AES.cpp
	g++ -I ../ -I ../../../extern/OpenSSL/include -I ../../../extern -w -c ../Net/Cryption/RSA.cpp
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/bcrypt.c
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_blowfish.c
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_gensalt.c
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/wrapper.c
endef

# link Library
define LINK
	ar rvs NetCore.a Net.o NetCodes.o NetVersion.o NetString.o XOR.o AES.o RSA.o bcrypt.o crypt_blowfish.o crypt_gensalt.o wrapper.o
endef

# copy Files
define COPY
	mkdir -p ../../bin/
	cp NetCore.a ../../bin/

	rm -r -f ../../bin/include/
	mkdir -p ../../bin/include/
	cp -r ../Net/ ../../bin/include/
	find ../../bin/include/ ! -name '*.h' -type f -exec rm -f {} +

	# copy crypto++
	rm -r -f ../../bin/include/crypto++/includes/
	mkdir -p ../../bin/include/crypto++/includes/
	cp -r ../../../extern/crypto++/includes/* ../../bin/include/crypto++/includes/

	# copy openssl
	rm -r -f ../../bin/include/openssl/
	cp -r ../../../extern/OpenSSL/include/ ../../bin/
endef

# clean build
define CLEAN
	rm *.o
	find ../../ -type f -name '*.h.gch' -delete
endef

all:
	$(call PATH_NET)
	$(call PATH_CRYPTION)
	$(call LINK)
	$(call COPY)
	$(call CLEAN)
