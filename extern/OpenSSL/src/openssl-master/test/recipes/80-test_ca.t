#! /usr/bin/env perl
# Copyright 2015-2020 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html


use strict;
use warnings;

use POSIX;
use File::Path 2.00 qw/rmtree/;
use OpenSSL::Test qw/:DEFAULT cmdstr srctop_file/;
use OpenSSL::Test::Utils;

setup("test_ca");

$ENV{OPENSSL} = cmdstr(app(["openssl"]), display => 1);

my $cnf = '"' . srctop_file("test","ca-and-certs.cnf") . '"';;
my $std_openssl_cnf = '"'
    . srctop_file("apps", $^O eq "VMS" ? "openssl-vms.cnf" : "openssl.cnf")
    . '"';

rmtree("demoCA", { safe => 0 });

plan tests => 6;
 SKIP: {
     $ENV{OPENSSL_CONFIG} = '-config ' . $cnf;
     skip "failed creating CA structure", 4
	 if !ok(run(perlapp(["CA.pl","-newca"], stdin => undef)),
		'creating CA structure');

     $ENV{OPENSSL_CONFIG} = '-config ' . $cnf;
     skip "failed creating new certificate request", 3
	 if !ok(run(perlapp(["CA.pl","-newreq",
                             '-extra-req', '-outform DER -section userreq'])),
		'creating certificate request');
     $ENV{OPENSSL_CONFIG} = '-rand_serial -inform DER -config '.$std_openssl_cnf;
     skip "failed to sign certificate request", 2
	 if !is(yes(cmdstr(perlapp(["CA.pl", "-sign"]))), 0,
		'signing certificate request');

     ok(run(perlapp(["CA.pl", "-verify", "newcert.pem"])),
        'verifying new certificate');

     skip "CT not configured, can't use -precert", 1
         if disabled("ct");

     $ENV{OPENSSL_CONFIG} = '-config ' . $cnf;
     ok(run(perlapp(["CA.pl", "-precert", '-extra-req', '-section userreq'], stderr => undef)),
        'creating new pre-certificate');
}

SKIP: {
    skip "SM2 is not supported by this OpenSSL build", 1
	      if disabled("sm2");

    is(yes(cmdstr(app(["openssl", "ca", "-config",
                       $cnf,
                       "-in", srctop_file("test", "certs", "sm2-csr.pem"),
                       "-out", "sm2-test.crt",
                       "-sigopt", "distid:1234567812345678",
                       "-vfyopt", "distid:1234567812345678",
                       "-md", "sm3",
                       "-cert", srctop_file("test", "certs", "sm2-root.crt"),
                       "-keyfile", srctop_file("test", "certs", "sm2-root.key")]))),
       0,
       "Signing SM2 certificate request");
}

sub yes {
    my $cntr = 10;
    open(PIPE, "|-", join(" ",@_));
    local $SIG{PIPE} = "IGNORE";
    1 while $cntr-- > 0 && print PIPE "y\n";
    close PIPE;
    return 0;
}

