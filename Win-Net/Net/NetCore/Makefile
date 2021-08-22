# build NetCore
CFLAGS=-std=c++11 -Wall -w
INCLUDE_PATH=-I ../ -I ../../../extern/ -I ../../../extern/BCrypt -I ../../../extern/crypto++/bin/include/ -I ../../../extern/OpenSSL/bin/bin/include

# Net/Net/
define PATH_NET
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Net/Net.cpp -o bin/Net.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Net/NetCodes.cpp -o bin/NetCodes.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Net/NetVersion.cpp -o bin/NetVersion.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Net/NetString.cpp -o bin/NetString.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Net/Package.cpp -o bin/Package.o
endef

# Net/Cryption/
define PATH_CRYPTION
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/XOR.cpp -o bin/XOR.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/PointerCryption.h -o bin/PointerCryption.h.gch
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/AES.cpp -o bin/AES.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/RSA.cpp -o bin/RSA.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/BCrypt/bcrypt.c -o bin/bcrypt.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/BCrypt/crypt_blowfish.c -o bin/crypt_blowfish.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/BCrypt/crypt_gensalt.c -o bin/crypt_gensalt.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Cryption/BCrypt/wrapper.c -o bin/wrapper.o
endef

# Net/Compression/
define PATH_COMPRESSION
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Compression/Compression.cpp -o bin/Compression.o
endef

# Net/assets/manager
define PATH_MANAGER
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/assets/manager/dirmanager.cpp -o bin/dirmanager.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/assets/manager/filemanager.cpp -o bin/filemanager.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/assets/manager/logmanager.cpp -o bin/logmanager.o
	g++ $(CFLAGS) -x c++ $(INCLUDE_PATH) -c ../Net/assets/manager/profile.hpp -o bin/profile.o
endef

# Net/assets/web
define PATH_WEB
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/assets/web/http.cpp -o bin/http.o
endef

# Net/assets
define PATH_ASSETS
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/assets/assets.cpp -o bin/assets.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/assets/thread.cpp -o bin/thread.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/assets/timer.cpp -o bin/timer.o
endef

# Net/Coding
define PATH_CODING
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Coding/BASE32.cpp -o bin/BASE32.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Coding/BASE64.cpp -o bin/BASE64.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -I ../../../extern/crypto++/bin/include/ -c ../Net/Coding/Hex.cpp -o bin/Hex.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Coding/MD5.cpp -o bin/MD5.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Coding/SHA1.cpp -o bin/SHA1.o
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Coding/TOTP.cpp -o bin/TOTP.o
endef

# Net/Protocol
define PATH_PROTOCOL
	g++ $(CFLAGS) $(INCLUDE_PATH) -c ../Net/Protocol/NTP.cpp -o bin/NTP.o
endef

# Net/Database
define PATH_DATABASE
	g++ $(CFLAGS) -D_GLIBCXX_USE_CXX11_ABI=0 $(INCLUDE_PATH) -c ../Net/Database/MYSQL.cpp -o bin/MYSQL.o
endef

# pugiXML
define PUGIXML
	g++ $(CFLAGS) -c ../../../extern/pugiXML/Unix/pugixml.cpp --output bin/pugixml.o
endef

# extract files from crypto++
define EXTRACT_CRYPTOPP
	cd bin/cryptopp && ar x ../../../../../extern/crypto++/bin/lib/libcryptopp.a
endef

# extract files from openssl
define EXTRACT_OPENSSL
	cd bin/openssl/libcrypto && ar x ../../../../../../extern/OpenSSL/bin/bin/lib/libcrypto.a
	cd bin/openssl/libssl && ar x ../../../../../../extern/OpenSSL/bin/bin/lib/libssl.a
endef

# extract files from mysql
define EXTRACT_MYSQL
	cd bin/mysql && ar x ../../../../../extern/MYSQL/lib/libmysqlcppconn-static.a
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
	find ../../bin/include/ ! -name '*.h' ! -name '*.hpp' -type f -exec rm -f {} +

	cp -r ../../../extern/JSON/ ../../bin/include/
 	mkdir -p ../../bin/include/MYSQL
 	mkdir -p ../../bin/include/MYSQL/include
	cp -r ../../../extern/MYSQL/include/* ../../bin/include/MYSQL/include
	cp -r ../../../extern/OpenSSL/bin/bin/include ../../bin/
	cp -r ../../../extern/crypto++/bin/include/ ../../bin/
	mkdir -p ../../bin/include/BCrypt
	cp -r ../../../extern/BCrypt/* ../../bin/include/BCrypt/
	mkdir -p ../../bin/include/pugiXML
	cp -r ../../../extern/pugiXML/Unix/*.hpp ../../bin/include/pugiXML/
	mkdir -p ../../bin/include/ZLib
	cp -r ../../../extern/ZLib/*.h ../../bin/include/ZLib/
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
	$(call PATH_CRYPTION)
	$(call PATH_COMPRESSION)
	$(call PATH_PROTOCOL)
	$(call PATH_DATABASE)

	$(call PUGIXML)

	$(call EXTRACT_CRYPTOPP)
	$(call EXTRACT_OPENSSL)
	$(call EXTRACT_MYSQL)

	$(call LINK)
	$(call COPY)
	$(call CLEAN)

