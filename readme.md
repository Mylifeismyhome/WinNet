	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
---
<br/>

## <span style="color:purple;font-weight: 600">[DEPENDENCIES]</span>
> - OpenSSL
> - Crypto++
> - ZLib
> - MySQL c++ connector by Oracle

<br/>

---

<br/>

## <span style="color:purple;font-weight: 600">[BUILD OPTIONS]</span>

> ## Windows
> - [x] Static Build
> - [ ] Shared Build
> - [ ] DLL Only Build
		
> ## Linux
> - [x] Static Build
> - [ ] Shared Build

<br/>

---

<br/>

## <span style="color:purple;font-weight: 600">[HOW TO BUILD IT]</span>
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
> ### build all
> - make all (includes dependencies)
> - make build
> - make sandbox
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
> - make netwebsocket	
> ### build sandbox
> - make sandboxclient
> - make sandboxserver
> - make sandboxwebsocket
> - make sandboxfeatures (use it to test functionality of NetCore Features)

<br/>

---

<br/>

## <span style="color:purple;font-weight: 600">[FEATURES]</span>
>> ## TCP ONLY
>> #### using WinSocket for Windows build and Unix Socket for Linux build 
>> - [x] Client
>> - [x] Server
>> - [x] Websocket
>> - [x] Peer Thread Pooling (definable amount of allowed peers inside a thread)
>> - [x] Non-Blocking
>
>> ## Classes
>>> ## Net
>>> - [x] Http/s
>>> - [x] String
>>> - [x] Json
>>> - [x] Packet
>>
>>> ## Database
>>> - [x] Mysql
>>
>>> ## Cryption
>>> - [x] AES
>>> - [x] RSA
>>> - [x] Pointer
>>> - [x] XOR Runtime & Compiletime
>>
>>> ## Compression
>>> - [x] ZLib
>>
>>> ## Coding
>>> - [x] Base32
>>> - [x] Base64
>>> - [x] Hex
>>> - [x] MD5
>>> - [x] SHA1
>>
>>> ## Auth
>>> - [x] TOTP
>>
>>> ## Manager
>>> - [x] Directory
>>> - [x] File
>>> - [x] Logging
>>
>>> ## Handler
>>> - [x] Thread
>>> - [x] Timer
>>
>> ## Implemented Protocols
>> - [x] NTP
>> - [x] ICMP
>
>> ## Windows
>>> - [x] Import Resolver
>
>> ## Communication between Server & Client
>> - [x] Plain
>> - [x] Crypted using RSA + AES
>> - [ ] Compression
>> - [x] TOTP to abort incoming packets that reached the defined allowed time
>> - [x] Json for data sharing
>> - [x] Raw bytes for file sharing
>
>> ## Websocket
>> - [x] Plain
>> - [x] TLS
