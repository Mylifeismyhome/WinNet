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
	${MAKE} static dynamic -C ${ROOT_DIR}/extern/crypto++/src/
	mkdir ${ROOT_DIR}/extern/crypto++/bin/
	mkdir ${ROOT_DIR}/extern/crypto++/bin/include/
	cp ${ROOT_DIR}/extern/crypto++/src/*.h ${ROOT_DIR}/extern/crypto++/bin/include/
	cp ${ROOT_DIR}/extern/crypto++/src/libcryptopp.so ${ROOT_DIR}/extern/crypto++/bin/libcryptopp.so
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

# build openssl
define openssl
	$(call out, ********** BUILDING OPENSSL **********)
	${ROOT_DIR}/extern/OpenSSL/src/openssl-master/config -fPIC shared --prefix=/usr --openssldir=/etc/ssl
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/
	${MAKE} install INSTALL_PREFIX=${ROOT_DIR}/extern/OpenSSL/src/BIN/ -C ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/
	mkdir ${ROOT_DIR}/extern/OpenSSL/lib/
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libssl.so ${ROOT_DIR}/extern/OpenSSL/lib/libssl.so
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libssl.a ${ROOT_DIR}/extern/OpenSSL/lib/libssl.a
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libcrypto.so ${ROOT_DIR}/extern/OpenSSL/lib/libcrypto.so
	cp ${ROOT_DIR}/extern/OpenSSL/src/openssl-master/libcrypto.a ${ROOT_DIR}/extern/OpenSSL/lib/libcrypto.a
endef

# clean all
define clean_all
	$(call out, ********** CLEANING ALL **********)
	$(call clean_crypto++)
endef

# run task
all:
	$(call crypto++)
	$(call openssl)

openssl:
	$(call openssl)

cryptopp:
	$(call crypto++)

crypto++:
	$(call crypto++)

clean_crypto++:
	$(call clean_crypto++)

clean:
	$(call clean_all)
# end
