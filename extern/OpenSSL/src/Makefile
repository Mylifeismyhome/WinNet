
export ROOT_DIR=${PWD}/extern/OpenSSL/src
OPENSSL_SOURCE_DIRNAME=openssl-1.1.1k

download:
	wget -O ${ROOT_DIR}/openssl-source.tar.gz "https://www.openssl.org/source/$(OPENSSL_SOURCE_DIRNAME).tar.gz" 

extract:
	rm -rf ${ROOT_DIR}/$(OPENSSL_SOURCE_DIRNAME)
	tar -xzf ${ROOT_DIR}/openssl-source.tar.gz

configure:
	cd ${ROOT_DIR}/$(OPENSSL_SOURCE_DIRNAME) && ./config -fPIC shared --prefix=${ROOT_DIR}/../bin/bin/ --openssldir=${ROOT_DIR}/../bin/openssl/

build:
	${MAKE} -C ${ROOT_DIR}/$(OPENSSL_SOURCE_DIRNAME)

install:
	${MAKE} -C ${ROOT_DIR}/$(OPENSSL_SOURCE_DIRNAME) install INSTALL_PREFIX=${ROOT_DIR}/bin/
