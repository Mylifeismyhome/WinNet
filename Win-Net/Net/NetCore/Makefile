# build NetCore
export ROOT_DIR=$(PWD)

# Net/Net/
define PATH_NET
	g++ -w -I ../ -I ../../../extern/OpenSSL/include -c ../Net/Net/Net.cpp -o bin/Net.o
	g++ -w -I ../ -c ../Net/Net/NetCodes.cpp -o bin/NetCodes.o
	g++ -w -I ../ -c ../Net/Net/NetVersion.cpp -o bin/NetVersion.o
	g++ -w -I ../ -c ../Net/Net/NetString.cpp -o bin/NetString.o
endef

# Net/Cryption/
define PATH_CRYPTION
	g++ -w -I ../ -c ../Net/Cryption/XOR.cpp -o bin/XOR.o
	g++ -I ../ -w -c ../Net/Cryption/PointerCryption.h -o bin/PointerCryption.h.gch
	g++ -I ../ -I ../../../extern/crypto++/bin/include/ -w -c ../Net/Cryption/AES.cpp -L ../../../extern/crypto++/bin/lib/ -l:libcryptopp.a -o bin/AES.o
	g++ -I ../ -I ../../../extern/OpenSSL/include -I ../../../extern -w -c ../Net/Cryption/RSA.cpp -o bin/RSA.o
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/bcrypt.c -o bin/bcrypt.o
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_blowfish.c -o bin/crypt_blowfish.o
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_gensalt.c -o bin/crypt_gensalt.o
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/wrapper.c -o bin/wrapper.o
endef

# Net/assets/manager
define PATH_MANAGER
	g++ -I ../ -w -c ../Net/assets/manager/dirmanager.cpp -o bin/dirmanager.o
	g++ -I ../ -w -c ../Net/assets/manager/filemanager.cpp -o bin/filemanager.o
	g++ -I ../ -w -c ../Net/assets/manager/logmanager.cpp -o bin/logmanager.o
endef

# Net/assets
define PATH_ASSETS
	g++ -I ../ -w -c ../Net/assets/assets.cpp -o bin/assets.o
endef

# Net/Coding
define PATH_CODING
	g++ -I ../ -w -c ../Net/Coding/BASE32.cpp -o bin/BASE32.o
	g++ -I ../ -w -c ../Net/Coding/BASE64.cpp -o bin/BASE64.o
	g++ -I ../ -I ../../../extern/crypto++/bin/include/ -w -c ../Net/Coding/Hex.cpp -L ../../../extern/crypto++/bin/lib/ -l:libcryptopp.a -o bin/Hex.o
	g++ -I ../ -w -c ../Net/Coding/MD5.cpp -o bin/MD5.o
	g++ -I ../ -w -c ../Net/Coding/SHA1.cpp -o bin/SHA1.o
	g++ -I ../ -w -c ../Net/Coding/TOTP.cpp -o bin/TOTP.o
endef

# extract files from crypto++
define EXTRACT_CRYPTOPP
	ar x ../../../extern/crypto++/bin/lib/libcryptopp.a --output bin/cryptopp/
endef

# extract files from openssl
define EXTRACT_OPENSSL
	ar x ../../../extern/OpenSSL/bin/bin/lib/libcrypto.a --output bin/openssl/libcrypto
	ar x ../../../extern/OpenSSL/bin/bin/lib/libssl.a --output bin/openssl/libssl
endef

# link Library
define LINK
	ar rvs NetCore.a bin/*.o bin/*.h.gch bin/cryptopp/*.o bin/openssl/libcrypto/*.o bin/openssl/libssl/*.o
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
	rm -r bin/
endef

all:
	mkdir -p bin
	mkdir -p bin/cryptopp
	mkdir -p bin/openssl
	mkdir -p bin/openssl/libcrypto
	mkdir -p bin/openssl/libssl

	$(call PATH_NET)
	$(call PATH_MANAGER)
	$(call PATH_ASSETS)
	$(call PATH_CODING)
	$(call PATH_CRYPTION)

	$(call EXTRACT_CRYPTOPP)
	$(call EXTRACT_OPENSSL)

	$(call LINK)
	$(call COPY)
	$(call CLEAN)
