# build NetCore
export ROOT_DIR=$(PWD)

# Net/Net/
define PATH_NET
	g++ -w -I ../ -I ../../../extern/OpenSSL/include -c ../Net/Net/Net.cpp -o bin/Net.o
	g++ -w -I ../ -c ../Net/Net/NetCodes.cpp -o bin/NetCodes.o
	g++ -w -I ../ -c ../Net/Net/NetVersion.cpp -o bin/NetVersion.o
	g++ -w -I ../ -c ../Net/Net/NetString.cpp -o bin/NetString.o
	g++ -w -I ../ -I ../../../extern/ -c ../Net/Net/Package.cpp -o bin/Package.o
endef

# Net/Cryption/
define PATH_CRYPTION
	g++ -w -I ../ -c ../Net/Cryption/XOR.cpp -o bin/XOR.o
	g++ -I ../ -w -c ../Net/Cryption/PointerCryption.h -o bin/PointerCryption.h.gch
	g++ -I ../ -I ../../../extern/crypto++/bin/include/ -w -c ../Net/Cryption/AES.cpp -L ../../../extern/crypto++/bin/lib/ -l:libcryptopp.a -o bin/AES.o
	g++ -I ../ -I ../../../extern/OpenSSL/include/ -I ../../../extern/crypto++/bin/include/ -w -c ../Net/Cryption/RSA.cpp -L ../../../extern/crypto++/bin/lib/ -L ../../../extern/OpenSSL/bin/bin/lib/ -l:libcryptopp.a -l:libcrypto.a -l:libssl.a -o bin/RSA.o -pthread -ldl
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/bcrypt.c -o bin/bcrypt.o
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_blowfish.c -o bin/crypt_blowfish.o
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/crypt_gensalt.c -o bin/crypt_gensalt.o
	g++ -I ../ -I ../../../extern/BCrypt -w -c ../Net/Cryption/BCrypt/wrapper.c -o bin/wrapper.o
endef

# Net/Compression/
define PATH_COMPRESSION
	g++ -w -I ../ -I ../../../extern/ -c ../Net/Compression/Compression.cpp -o bin/Compression.o
endef

# Net/assets/manager
define PATH_MANAGER
	g++ -I ../ -w -c ../Net/assets/manager/dirmanager.cpp -o bin/dirmanager.o
	g++ -I ../ -w -c ../Net/assets/manager/filemanager.cpp -o bin/filemanager.o
	g++ -I ../ -w -c ../Net/assets/manager/logmanager.cpp -o bin/logmanager.o
	g++ -I ../ -I ../../../extern/ -w -c ../Net/assets/manager/profile.hpp -o bin/profile.o
endef

# Net/assets/web
define PATH_WEB
	g++ -I ../ -I ../../../extern/OpenSSL/include/ -w -c ../Net/assets/web/http.cpp -o bin/http.o
endef

# Net/assets
define PATH_ASSETS
	g++ -I ../ -w -c ../Net/assets/assets.cpp -o bin/assets.o
	g++ -I ../ -w -c ../Net/assets/thread.cpp -o bin/thread.o
	g++ -I ../ -w -c ../Net/assets/timer.cpp -o bin/timer.o
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

# Net/Database
define PATH_DATABASE
	g++ -std=c++11 -I ../ -I ../../../extern/ -w -c ../Net/Database/MYSQL.cpp -L ../../../extern/MYSQL/lib/ -l:libmysqlcppconn-static.a -o bin/MYSQL.o
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

# extract files from mysql
define EXTRACT_MYSQL
	ar x ../../../extern/MYSQL/lib/libmysqlcppconn-static.a --output bin/mysql
endef

# link Library
define LINK
	ar rvs NetCore.a bin/*.o bin/*.h.gch bin/cryptopp/*.o bin/openssl/libcrypto/*.o bin/openssl/libssl/*.o bin/mysql/*.o
	ranlib NetCore.a
endef

# copy Files
define COPY
	mkdir -p ../../bin/
	cp NetCore.a ../../bin/

	rm -r -f ../../bin/include/
	mkdir -p ../../bin/include/
	cp -r ../Net/ ../../bin/include/
	find ../../bin/include/ ! -name '*.h' -type f -exec rm -f {} +
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
	mkdir -p bin/mysql

	$(call PATH_NET)
	$(call PATH_MANAGER)
	$(call PATH_WEB)
	$(call PATH_ASSETS)
	$(call PATH_CODING)
	$(call PATH_DATABASE)
	$(call PATH_CRYPTION)
	$(call PATH_COMPRESSION)

	$(call EXTRACT_CRYPTOPP)
	$(call EXTRACT_OPENSSL)
	$(call EXTRACT_MYSQL)

	$(call LINK)
	$(call COPY)
	$(call CLEAN)
