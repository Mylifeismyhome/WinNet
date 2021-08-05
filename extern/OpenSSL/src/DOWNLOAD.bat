@echo off
title Downloading OpenSSL from Github - [%date% %time%]
set /p OPENSSL_SOURCE_DIRNAME=<openssl-version

curl -L https://www.openssl.org/source/%OPENSSL_SOURCE_DIRNAME%.tar.gz --output openssl-source.tar.gz