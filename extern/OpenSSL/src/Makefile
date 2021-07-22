export ROOT_DIR=${PWD}

all:
	${ROOT_DIR}/openssl-master/config -fPIC shared --prefix=/usr --openssldir=/etc/ssl
	${MAKE} -C ${ROOT_DIR}/openssl-master/
	${MAKE} install INSTALL_PREFIX=${ROOT_DIR}/bin/ -C ${ROOT_DIR}/openssl-master/
