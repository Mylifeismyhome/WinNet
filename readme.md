Author: Tobias Staack

	[LINUX]
		# build dependencies
		make cryptopp
		make openssl-download
		make openssl-unpack
		make openssl-configure
		make openssl-build
		make openssl-install
		make mysql-download
		make mysql-install
		
		# clean
		make cryptopp-clean
		make openssl-clean
		make mysql-clean
		
		# clean all
		make clean
		
		# build
		make netcore
		make netclient
		make netserver
		make netwebserver
		
		# build sandbox
		make sandboxclient
		make sandboxserver
		make sandboxwebserver
	
	[WINDOWS]
	Requires:
		nasm
		perl
		vc build tools
	
	MYSQL:
		download mysql c++ connector dependencies and place all files in:
		MYSQL/lib/x64/release/
		MYSQL/lib/x64/debug/
		MYSQL/lib/x86/release/
		MYSQL/lib/x86/debug/
		
	Install & Compile:
		run INSTALL.bat
			> if fails on downloading perl or nasm, manually install them and continue.
			> enter vc path (as example: C:\Program Files (x86)\Microsoft Visual Studio\2019)
			> enter perl path (as example: C:\Strawberry)
			> enter nasm path (as example: C:\Users\Tobias\AppData\Local\bin\NASM)
			> [WARNING] it uses 'setx' to set nasm path to system environment.
			> this batch automatically runs BUILD.bat
	
	[COMPILATION]
		- Windows
- [x] Static Build
- [ ] Shared Build
- [ ] DLL Only Build
		
		- Linux
			[X] Static Build
  			[ ] Shared Build
