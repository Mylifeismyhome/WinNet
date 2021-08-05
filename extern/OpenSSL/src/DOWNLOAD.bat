@echo off
title Downloading OpenSSL from Github - [%date% %time%]

curl -L https://github.com/openssl/openssl/archive/refs/heads/master.zip --output openssl-master.zip
