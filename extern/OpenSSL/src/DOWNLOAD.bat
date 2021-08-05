@echo off
title Downloading OpenSSL from Github - [%date% %time%]
set OPENSSL_SOURCE_DIRNAME=openssl-1.1.1k

curl -L https://www.openssl.org/source/%OPENSSL_SOURCE_DIRNAME%.tar.gz --output openssl-source.tar.gz