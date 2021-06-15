export ROOT_DIR=${PWD}
red:=$(shell tput setaf 1)
reset:=$(shell tput sgr0)

# console message
define out
    $(info $(red)$(1)$(reset))
endef

# build crypto++
define crypto++
	${MAKE} static dynamic -C ${ROOT_DIR}/extern/crypto++/src/
endef

# build openssl
define openssl
	${ROOT_DIR}/extern/OpenSSL/src/openssl-master/config -fPIC shared --prefix=/usr --openssldir=/etc/ssl
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/
	${MAKE} install INSTALL_PREFIX=${ROOT_DIR}/extern/OpenSSL/src/BIN/ -C ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/
	mkdir ${ROOT_DIR}/extern/OpenSSL/lib/
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libssl.so ${ROOT_DIR}/extern/OpenSSL/lib/libssl.so
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libssl.a ${ROOT_DIR}/extern/OpenSSL/lib/libssl.a
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libcrypto.so ${ROOT_DIR}/extern/OpenSSL/lib/libcrypto.so
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libcrypto.a ${ROOT_DIR}/extern/OpenSSL/lib/libcrypto.a
endef

# run task
all:
	$(call out, ********** RUNNING BUILD **********)
	$(call crypto++)
	$(call openssl)
	$(call end)

# end
define end
endef
