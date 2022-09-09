### Author: *Tobias Staack*
---
<br/>

## <span style="color:purple;font-weight: 600">[COMPILATION]</span>

> ## Windows
> - [x] Static Build
> - [ ] Shared Build
> - [ ] DLL Only Build
		
> ## Linux
> - [x] Static Build
> - [ ] Shared Build
---
## <span style="color:purple;font-weight: 600">[BUILD]</span>
> ## Windows
> ### Requires
> - nasm
> - perl
> - vc build tools
> ### MYSQL
> #### To compile WinNet with MYSQL Support you will have to download mysql c++ connector and place all files in:
> - MYSQL/lib/x64/release/
> - MYSQL/lib/x64/debug/
> - MYSQL/lib/x86/release/
> - MYSQL/lib/x86/debug/
> ### Install and Compile
> 1. Run Install.bat inside of Scripts folder
> 2. Enter VC Path (as example: C:\Program Files (x86)\Microsoft Visual Studio\2019)
> 3. Enter perl path (as example: C:\Strawberry)
> 4. Enter nasm path (as example: C:\Users\Tobias\AppData\Local\bin\NASM)
>> ## <span style="color:red;font-weight: 800">[WARNING]</span>
>> it uses '<span style="color:yellow;font-weight: 400">setx</span>' to set nasm path to system environment.
this batch automatically runs BUILD.bat

> ## Linux
> ### build dependencies
> - make cryptopp
> - make openssl-download
> - make openssl-unpack
> - make openssl-configure
> - make openssl-build
> - make openssl-install
> - make mysql-download
> - make mysql-install	
> ### clean
> - make cryptopp-clean
> - make openssl-clean
> - make mysql-clean	
> ### clean all
> - make clean		
> ### build
> - make netcore
> - make netclient
> - make netserver
> - make netwebserver		
> ### build sandbox
> - make sandboxclient
> - make sandboxserver
> - make sandboxwebserver
