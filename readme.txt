/*
Author: Tobias Staack
*/

How To Compile Dependencies:
 Crypto++:
    Just run batch files
    Make sure you have set the correct path to run vcbuildtools_msbuild.bat in each batch file

  OpenSSL:
    requires nasm
    requires Perl
    requires nmake and nasm path variable

    change vs paths in each batch if required
    and run batch files

  MYSQL:
    download mysql c++ connector dependencies and place all files in:
    MYSQL/lib/x64/release/
    MYSQL/lib/x64/debug/
    MYSQL/lib/x86/release/
    MYSQL/lib/x86/debug/