@echo off
title Unpacking OpenSSL Archive - [%date% %time%]
set OPENSSL_SOURCE_DIRNAME=openssl-1.1.1k

rmdir %OPENSSL_SOURCE_DIRNAME% /s /q
tar -xzf openssl-source.tar.gz