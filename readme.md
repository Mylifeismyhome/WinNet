/*
Author: Tobias Staack
*/

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
	