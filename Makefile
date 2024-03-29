export ROOT_DIR=${PWD}
red:=$(shell tput setaf 1)
reset:=$(shell tput sgr0)

# console message
define out
    $(info $(red)$(1)$(reset))
endef

# build crypto++
define crypto++
	$(call out, ********** BUILDING CRYPTO++ **********)

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
	$(call out, ********** DELETING FILES CREATED BY CRYPTO++ **********)

	rm -f ${ROOT_DIR}/extern/crypto++/src/*.o
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.so
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.so.*
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.a
	rm -f ${ROOT_DIR}/extern/crypto++/src/*.out
endef

# download openssl
define download_openssl
        $(call out, ********** DOWNLOADING OPENSSL REPO FROM GITHUB **********)
        ${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ download
endef

# extract openssl
define extract_openssl
        $(call out, ********** UNPACKING OPENSSL REPO **********)
        ${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ extract
endef

# configure openssl
define configure_openssl
	$(call out, ********** CONFIGURING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ configure
endef

# build openssl
define build_openssl
	$(call out, ********** BUILDING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -i -k build
endef

# install openssl
define install_openssl
	$(call out, ********** INSTALLING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -i -k install
endef

# clean openssl
define clean_openssl
	$(call out, ********** DELETING OPENSSL **********)
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/openssl-source.tar.gz
endef

# download compressed (packaged) mysql library from the official website
define download_mysql
	wget -O ${ROOT_DIR}/extern/MYSQL/mysql_package.tar.gz "https://dev.mysql.com/get/Downloads/Connector-C++/mysql-connector-c++-8.0.26-linux-glibc2.12-x86-64bit.tar.gz"
endef

# install mysql library
define install_mysql
	mkdir -p ${ROOT_DIR}/extern/MYSQL/tmp/
	tar zxvf ${ROOT_DIR}/extern/MYSQL/mysql_package.tar.gz -C ${ROOT_DIR}/extern/MYSQL/tmp/
	cp -R ${ROOT_DIR}/extern/MYSQL/tmp/*/include/* ${ROOT_DIR}/extern/MYSQL/include/
	mkdir -p ${ROOT_DIR}/extern/MYSQL/lib/
	cp ${ROOT_DIR}/extern/MYSQL/tmp/*/lib64/libmysqlcppconn-static.a ${ROOT_DIR}/extern/MYSQL/lib/
endef

# clean mysql installation
define clean_mysql
	# installation completed, now delete tmp folder
        rm -r ${ROOT_DIR}/extern/MYSQL/tmp/

        # also get rid of the package
        rm ${ROOT_DIR}/extern/MYSQL/mysql_package.tar.gz
endef

# clean all
define clean_all
	$(call out, ********** CLEANING ALL **********)
	$(call clean_crypto++)
	$(call clean_openssl)
	$(call clean_mysql)
endef

# run task
all:
	$(call crypto++)
	$(call openssl)

openssl-download:
	$(call download_openssl)

openssl-unpack:
	$(call extract_openssl)

openssl-configure:
	$(call configure_openssl)

openssl-build:
	$(call build_openssl)

openssl-install:
	$(call install_openssl)

cryptopp:
	$(call crypto++)

crypto++:
	$(call crypto++)

cryptopp-clean:
	$(call clean_crypto++)

crypto++-clean:
	$(call clean_crypto++)

openssl-clean:
	$(call clean_openssl)

mysql-download:
	$(call download_mysql)

mysql-install:
	$(call install_mysql)

mysql-clean:
	$(call clean_mysql)

netcore:
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetCore/

netserver:
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetServer/

netclient:
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetClient/

netwebserver:
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetWebServer/

sandboxserver:
	${MAKE} -C ${ROOT_DIR}/Sandbox/Server/Server/

sandboxclient:
	${MAKE} -C ${ROOT_DIR}/Sandbox/Client/Client/

sandboxwebserver:
	${MAKE} -C ${ROOT_DIR}/Sandbox/WebServer/WebServer/

clean:
	$(call clean_all)
# end
