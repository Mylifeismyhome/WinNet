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

## **BUILD HIERARCHY**
### NetCore
- OpenSLL
- Crypto++
- ZLib
- MySQL C++ Connector By Orcale

### NetClient
- NetCore

### NetServer
- NetCore

### NetWebSocket
- NetCore

---

## **BUILD OPTIONS**
### Windows
- [x] Static Build
- [ ] Shared Build
- [ ] DLL Only Build
		
### Linux
- [x] Static Build
- [ ] Shared Build

---

## **HOW TO BUILD**
## Windows
### Requires
- nasm
- perl
- vc build tools
### MYSQL
#### To compile WinNet with MYSQL Support you will have to download mysql c++ connector and place all files in:
- MYSQL/lib/x64/release/
- MYSQL/lib/x64/debug/
- MYSQL/lib/x86/release/
- MYSQL/lib/x86/debug/
### Install and Compile
1. Run Install.bat inside of Scripts folder
2. Enter VC Path (as example: C:\Program Files (x86)\Microsoft Visual Studio\2019)
3. Enter perl path (as example: C:\Strawberry)
4. Enter nasm path (as example: C:\Users\Tobias\AppData\Local\bin\NASM)
> ## <span style="color:red;font-weight: 800">[WARNING]</span>
> uses '<span style="color:yellow;font-weight: 400">setx</span>' to set nasm path to system environment.

## Linux
### build all
	make all (include dependencies)
	make build
	make sandbox
### build dependencies
	make cryptopp
	make openssl-download
	make openssl-unpack
	make openssl-configure
	make openssl-build
	make openssl-install
	make mysql-download
	make mysql-install	
### clean
	make cryptopp-clean
	make openssl-clean
	make mysql-clean	
### clean all
	make clean		
### build
	make netcore
	make netclient
	make netserver
	make netwebsocket	
### build sandbox
	make sandboxclient
	make sandboxserver
	make sandboxwebsocket
	make sandboxfeatures (use it to test functionality of NetCore Features)

---

## **FEATURES**
>### TCP ONLY
>>#### using WinSocket for Windows build and Unix Socket for Linux build
- [x] Client
- [x] Server
- [x] Websocket
- [x] Peer Thread Pooling (definable amount of allowed peers inside a thread)
- [x] Non-Blocking

## Classes
## Net
- [x] Http/s
- [x] String
- [x] Json
- [x] Packet
- [x] Thread
- [x] Timer
- [x] Pointer encryption (Runtime)
- [x] XOR-String encryption (Runtime & Compiletime)
- [x] Directory-Manager
- [x] File-Manager
- [x] Log-Manager

## Extern
- [x] MySQL (c++ connector by Oracle)
- [x] AES (OpenSSL)
- [x] RSA (CryptoPP)
- [x] ZLib
- [x] Base32
- [x] Base64
- [x] Hex (CryptoPP)
- [x] MD5
- [x] SHA1
- [x] TOTP (Time-Based one-time password)

## Proctol
- [x] NTP (Network Time Protocol)
- [x] ICMP (Internet Control Message Protocol)

## Windows
- [x] Custom import resolver

## Server & Client
- [x] Unsafe plain communication.
- [x] Safe hybrid-encrypted communication using RSA + AES combination.
- [x] Compression mode using zlib algorithmus.
- [x] JSON based packet data
- [x] Raw packet data for large amount of bytes
- [x] Ping measuring using ICMP

## Websocket
- [x] Unsafe plain communication
- [x] Safe communication using TLS (SSL)
- [x] Sanitize user input to prevent XSS
