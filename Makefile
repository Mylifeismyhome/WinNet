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

# configure openssl
define configure_openssl
	$(call out, ********** CONFIGURING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -f OpenSSL_Makefile configure
endef

# build openssl
define build_openssl
	$(call out, ********** BUILDING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -f OpenSSL_Makefile -i -k build
endef

# install openssl
define install_openssl
	$(call out, ********** INSTALLING OPENSSL **********)
	${MAKE} -C ${ROOT_DIR}/extern/OpenSSL/src/ -f OpenSSL_Makefile -i -k install
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

# build NetCore
define NetCore
	${MAKE} -C ${ROOT_DIR}/Win-Net/Net/NetCore/
endef

# run task
all:
	$(call crypto++)
	$(call openssl)

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

netcore:
	$(call NetCore)

clean:
	$(call clean_all)
# end
