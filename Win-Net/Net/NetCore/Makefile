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
	g++ -I ../ -I ../../../extern -L ../../../extern/crypto++/bin/ -w -c ../Net/Cryption/AES.cpp -l:libcryptopp.so
	g++ -I ../ -I ../../../extern/OpenSSL/include -I ../../../extern -w -c ../Net/Cryption/RSA.cpp
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/bcrypt.c
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_blowfish.c
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_gensalt.c
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/wrapper.c
endef

# Net/assets/manager
define PATH_MANAGER
	g++ -I ../ -w -c ../Net/assets/manager/dirmanager.cpp
	g++ -I ../ -w -c ../Net/assets/manager/filemanager.cpp
	g++ -I ../ -w -c ../Net/assets/manager/logmanager.cpp
endef

# Net/assets
define PATH_ASSETS
	g++ -I ../ -w -c ../Net/assets/assets.cpp
endef

# Net/Coding
define PATH_CODING
	g++ -I ../ -w -c ../Net/Coding/BASE32.cpp
	g++ -I ../ -w -c ../Net/Coding/BASE64.cpp
	g++ -I ../ -I ../../../extern -L ../../../extern/crypto++/bin/ -w -c ../Net/Coding/Hex.cpp -l:libcryptopp.so
	g++ -I ../ -w -c ../Net/Coding/MD5.cpp
	g++ -I ../ -w -c ../Net/Coding/SHA1.cpp
	g++ -I ../ -w -c ../Net/Coding/TOTP.cpp
endef

# link Library
define LINK
	ar rvs NetCore.a ../../../extern/crypto++/bin/libcryptopp.so Net.o NetCodes.o NetVersion.o NetString.o XOR.o AES.o RSA.o bcrypt.o crypt_blowfish.o crypt_gensalt.o wrapper.o dirmanager.o filemanager.o logmanager.o assets.o BASE32.o BASE64.o Hex.o MD5.o SHA1.o TOTP.o
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
	$(call PATH_MANAGER)
	$(call PATH_ASSETS)
	$(call PATH_CODING)
	$(call PATH_CRYPTION)
	$(call LINK)
	$(call COPY)
	$(call CLEAN)
