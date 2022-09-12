
export ROOT_DIR=${PWD}

# build crypto++
define crypto++
	@printf "`tput setaf 1`********** BUILDING CRYPTO++ **********`tput sgr0`\n"

	# build
	${MAKE} -C ${ROOT_DIR}/extern/crypto++/src/ static dynamic cryptest.exe

	# validate
	./extern/crypto++/src/cryptest.exe v
	./extern/crypto++/src/cryptest.exe tv all

	${MAKE} -C ${ROOT_DIR}/extern/crypto++/src/ libcryptopp.a libcryptopp.so cryptest.exe

	# create install directory
	mkdir -p ${ROOT_DIR}/extern/crypto++/bin/

	# install
	sudo ${MAKE} -C ${ROOT_DIR}/extern/crypto++/src/ install PREFIX=${ROOT_DIR}/extern/crypto++/bin/
endef

# clean crypto++
define clean_crypto++
      	@printf "`tput setaf 1`********** CLEANING UP CRYPTO++ FILES **********`tput sgr0`\n"

	rm -f ${ROOT_DIR}/extern/crypto++/src/*.o
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.so
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.so.*
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.a
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.out
endef

# download openssl
define download_openssl
        @printf "`tput setaf 1`********** DOWNLOADING OPENSSL REPO FROM GITHUB **********`tput sgr0`\n"
        ${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ download
endef

# extract openssl
define extract_openssl
        @printf "`tput setaf 1`********** UNPACKING OPENSSL REPO **********`tput sgr0`\n"
        ${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ extract
endef

# configure openssl
define configure_openssl
        @printf "`tput setaf 1`********** CONFIGURING OPENSSL **********`tput sgr0`\n"
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ configure
endef

# build openssl
define build_openssl
        @printf "`tput setaf 1`********** BUILDING OPENSSL **********`tput sgr0`\n"
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -i -k build
endef

# install openssl
define install_openssl
        @printf "`tput setaf 1`********** INSTALLING OPENSSL **********`tput sgr0`\n"
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -i -k install
endef

# clean openssl
define clean_openssl
        @printf "`tput setaf 1`********** CLEANING UP OPENSSL FILES **********`tput sgr0`\n"
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/openssl-source.tar.gz
endef

# download compressed (packaged) mysql library from the official website
define download_mysql
        @printf "`tput setaf 1`********** DOWNLOADING MYSQL (C++ CONNECTOR) **********`tput sgr0`\n"
	wget -O ${ROOT_DIR}/extern/MYSQL/mysql_package.tar.gz "https://dev.mysql.com/get/Downloads/Connector-C++/mysql-connector-c++-8.0.26-linux-glibc2.12-x86-64bit.tar.gz"
endef

# install mysql library
define install_mysql
        @printf "`tput setaf 1`********** INSTALLING MYSQL (C++ CONNECTOR) **********`tput sgr0`\n"

	mkdir -p ${ROOT_DIR}/extern/MYSQL/tmp/
	tar zxvf ${ROOT_DIR}/extern/MYSQL/mysql_package.tar.gz -C ${ROOT_DIR}/extern/MYSQL/tmp/
	cp -R ${ROOT_DIR}/extern/MYSQL/tmp/*/include/* ${ROOT_DIR}/extern/MYSQL/include/
	mkdir -p ${ROOT_DIR}/extern/MYSQL/lib/
	cp ${ROOT_DIR}/extern/MYSQL/tmp/*/lib64/libmysqlcppconn-static.a ${ROOT_DIR}/extern/MYSQL/lib/
endef

# clean mysql installation
define clean_mysql
        @printf "`tput setaf 1`********** CLEANING UP MYSQL FILES **********`tput sgr0`\n"

	# installation completed, now delete tmp folder
        rm -r ${ROOT_DIR}/extern/MYSQL/tmp/

        # also get rid of the package
        rm ${ROOT_DIR}/extern/MYSQL/mysql_package.tar.gz
endef

# clean all
define clean_all
        @printf "`tput setaf 1`********** CLEANING UP EVERYTHING **********`tput sgr0`\n"
	$(clean_crypto++)
	$(clean_openssl)
	$(clean_mysql)
endef

# build netcore
define build_netcore
        @printf "`tput setaf 1`********** BUILDING NETCORE **********`tput sgr0`\n"
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetCore/
endef

# build netclient
define build_netclient
        @printf "`tput setaf 1`********** BUILDING NETCLIENT **********`tput sgr0`\n"
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetClient/
endef

# build netserver
define build_netserver
        @printf "`tput setaf 1`********** BUILDING NETSERVER **********`tput sgr0`\n"
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetServer/
endef

# build netwebsocket
define build_netwebsocket
        @printf "`tput setaf 1`********** BUILDING NETWEBSOCKET **********`tput sgr0`\n"
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetWebSocket/
endef

# build sandboxclient
define build_sandboxclient
        @printf "`tput setaf 1`********** BUILDING SANDBOX CLIENT **********`tput sgr0`\n"
        ${MAKE} -C ${ROOT_DIR}/Sandbox/Client/Client/
endef

# build sandboxserver
define build_sandboxserver
        @printf "`tput setaf 1`********** BUILDING SANDBOX SERVER **********`tput sgr0`\n"
        ${MAKE} -C ${ROOT_DIR}/Sandbox/Server/Server/
endef

# build sandboxwebsocket
define build_sandboxwebsocket
        @printf "`tput setaf 1`********** BUILDING SANDBOX WEBSOCKET **********`tput sgr0`\n"
        ${MAKE} -C ${ROOT_DIR}/Sandbox/WebSocket/WebSocket/
endef

# run task
all:
	$(crypto++)
	$(download_openssl)
	$(extract_openssl)
	$(configure_openssl)
	$(build_openssl)
	$(install_openssl)
	$(download_mysql)
	$(install_mysql)
	$(clean_all)

	$(build_netcore)
	$(build_netclient)
	$(build_netserver)
	$(build_netwebsocket)

build:
	$(build_netcore)
	$(build_netclient)
	$(build_netserver)
	$(build_netwebsocket)

sandbox:
	$(build_sandboxclient)
	$(build_sandboxserver)
	$(build_sandboxwebsocket)

openssl-download:
	$(download_openssl)

openssl-unpack:
	$(extract_openssl)

openssl-configure:
	$(configure_openssl)

openssl-build:
	$(build_openssl)

openssl-install:
	$(install_openssl)

cryptopp:
	$(crypto++)

crypto++:
	$(crypto++)

cryptopp-clean:
	$(clean_crypto++)

crypto++-clean:
	$(clean_crypto++)

openssl-clean:
	$(clean_openssl)

mysql-download:
	$(download_mysql)

mysql-install:
	$(install_mysql)

mysql-clean:
	$(clean_mysql)

netcore:
	$(build_netcore)

netserver:
	$(build_netserver)

netclient:
	$(build_netclient)

netwebsocket:
	$(build_netwebsocket)

sandboxserver:
	$(build_sandboxserver)

sandboxclient:
	$(build_sandboxclient)

sandboxwebsocket:
	$(build_sandboxwebsocket)

clean:
	$(clean_all)
# end
