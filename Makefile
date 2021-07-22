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

# configure openssl
define configure_openssl
	$(call out, ********** CONFIGURING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -f OpenSSL_Makefile configure
endef

# build openssl
define build_openssl
	$(call out, ********** BUILDING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -f OpenSSL_Makefile build
endef

# install openssl
define install_openssl
	$(call out, ********** INSTALLING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -f OpenSSL_Makefile install
endef

# clean openssl
define clean_openssl
	$(call out, ********** DELETING FILES CREATED BY OPENSSL **********)
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/crypto/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/include/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/tools/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/engines/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/ms/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/ssl/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/util/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/fuzz/
	rm -f -R -d ${ROOT_DIR}/extern/OpenSSL/src/apps/
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/*.a
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/*.pm
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/*.so
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/*.pc
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/*.map
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/*.so.*
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/Makefile
	rm -f ${ROOT_DIR}/extern/OpenSSL/src/configdata.pm
endef

# clean all
define clean_all
	$(call out, ********** CLEANING ALL **********)
	$(call clean_crypto++)
	$(call clean_openssl)
endef

# run task
all:
	$(call crypto++)
	$(call openssl)

configure_openssl:
	$(call configure_openssl)

build_openssl:
	$(call build_openssl)

install_openssl:
	$(call install_openssl)

cryptopp:
	$(call crypto++)

crypto++:
	$(call crypto++)

clean_crypto++:
	$(call clean_crypto++)

clean_openssl:
	$(call clean_openssl)

clean:
	$(call clean_all)
# end
