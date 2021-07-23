# build NetCore
export ROOT_DIR=$(PWD)

# Net/Net/
define PATH_NET
	g++ -I ../ -c ../Net/Net/Net.cpp
endef

all:
	$(call PATH_NET)
	g++ -I ../ -c ../Net/Cryption/PointerCryption.h
