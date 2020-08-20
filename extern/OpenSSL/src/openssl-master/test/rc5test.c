/*
 * Copyright 1995-2017 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

/*
 * RC5 low level APIs are deprecated for public use, but still ok for internal
 * use.
 */
#include "internal/deprecated.h"

#include <string.h>

#include "internal/nelem.h"
#include "testutil.h"

#ifndef OPENSSL_NO_RC5
# include <openssl/rc5.h>

static unsigned char RC5key[5][16] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x91, 0x5f, 0x46, 0x19, 0xbe, 0x41, 0xb2, 0x51,
     0x63, 0x55, 0xa5, 0x01, 0x10, 0xa9, 0xce, 0x91},
    {0x78, 0x33, 0x48, 0xe7, 0x5a, 0xeb, 0x0f, 0x2f,
     0xd7, 0xb1, 0x69, 0xbb, 0x8d, 0xc1, 0x67, 0x87},
    {0xdc, 0x49, 0xdb, 0x13, 0x75, 0xa5, 0x58, 0x4f,
     0x64, 0x85, 0xb4, 0x13, 0xb5, 0xf1, 0x2b, 0xaf},
    {0x52, 0x69, 0xf1, 0x49, 0xd4, 0x1b, 0xa0, 0x15,
     0x24, 0x97, 0x57, 0x4d, 0x7f, 0x15, 0x31, 0x25},
};

static unsigned char RC5plain[5][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x21, 0xA5, 0xDB, 0xEE, 0x15, 0x4B, 0x8F, 0x6D},
    {0xF7, 0xC0, 0x13, 0xAC, 0x5B, 0x2B, 0x89, 0x52},
    {0x2F, 0x42, 0xB3, 0xB7, 0x03, 0x69, 0xFC, 0x92},
    {0x65, 0xC1, 0x78, 0xB2, 0x84, 0xD1, 0x97, 0xCC},
};

static unsigned char RC5cipher[5][8] = {
    {0x21, 0xA5, 0xDB, 0xEE, 0x15, 0x4B, 0x8F, 0x6D},
    {0xF7, 0xC0, 0x13, 0xAC, 0x5B, 0x2B, 0x89, 0x52},
    {0x2F, 0x42, 0xB3, 0xB7, 0x03, 0x69, 0xFC, 0x92},
    {0x65, 0xC1, 0x78, 0xB2, 0x84, 0xD1, 0x97, 0xCC},
    {0xEB, 0x44, 0xE4, 0x15, 0xDA, 0x31, 0x98, 0x24},
};

# define RC5_CBC_NUM 27
static unsigned char rc5_cbc_cipher[RC5_CBC_NUM][8] = {
    {0x7a, 0x7b, 0xba, 0x4d, 0x79, 0x11, 0x1d, 0x1e},
    {0x79, 0x7b, 0xba, 0x4d, 0x78, 0x11, 0x1d, 0x1e},
    {0x7a, 0x7b, 0xba, 0x4d, 0x79, 0x11, 0x1d, 0x1f},
    {0x7a, 0x7b, 0xba, 0x4d, 0x79, 0x11, 0x1d, 0x1f},
    {0x8b, 0x9d, 0xed, 0x91, 0xce, 0x77, 0x94, 0xa6},
    {0x2f, 0x75, 0x9f, 0xe7, 0xad, 0x86, 0xa3, 0x78},
    {0xdc, 0xa2, 0x69, 0x4b, 0xf4, 0x0e, 0x07, 0x88},
    {0xdc, 0xa2, 0x69, 0x4b, 0xf4, 0x0e, 0x07, 0x88},
    {0xdc, 0xfe, 0x09, 0x85, 0x77, 0xec, 0xa5, 0xff},
    {0x96, 0x46, 0xfb, 0x77, 0x63, 0x8f, 0x9c, 0xa8},
    {0xb2, 0xb3, 0x20, 0x9d, 0xb6, 0x59, 0x4d, 0xa4},
    {0x54, 0x5f, 0x7f, 0x32, 0xa5, 0xfc, 0x38, 0x36},
    {0x82, 0x85, 0xe7, 0xc1, 0xb5, 0xbc, 0x74, 0x02},
    {0xfc, 0x58, 0x6f, 0x92, 0xf7, 0x08, 0x09, 0x34},
    {0xcf, 0x27, 0x0e, 0xf9, 0x71, 0x7f, 0xf7, 0xc4},
    {0xe4, 0x93, 0xf1, 0xc1, 0xbb, 0x4d, 0x6e, 0x8c},
    {0x5c, 0x4c, 0x04, 0x1e, 0x0f, 0x21, 0x7a, 0xc3},
    {0x92, 0x1f, 0x12, 0x48, 0x53, 0x73, 0xb4, 0xf7},
    {0x5b, 0xa0, 0xca, 0x6b, 0xbe, 0x7f, 0x5f, 0xad},
    {0xc5, 0x33, 0x77, 0x1c, 0xd0, 0x11, 0x0e, 0x63},
    {0x29, 0x4d, 0xdb, 0x46, 0xb3, 0x27, 0x8d, 0x60},
    {0xda, 0xd6, 0xbd, 0xa9, 0xdf, 0xe8, 0xf7, 0xe8},
    {0x97, 0xe0, 0x78, 0x78, 0x37, 0xed, 0x31, 0x7f},
    {0x78, 0x75, 0xdb, 0xf6, 0x73, 0x8c, 0x64, 0x78},
    {0x8f, 0x34, 0xc3, 0xc6, 0x81, 0xc9, 0x96, 0x95},
    {0x7c, 0xb3, 0xf1, 0xdf, 0x34, 0xf9, 0x48, 0x11},
    {0x7f, 0xd1, 0xa0, 0x23, 0xa5, 0xbb, 0xa2, 0x17},
};

static unsigned char rc5_cbc_key[RC5_CBC_NUM][17] = {
    {1, 0x00},
    {1, 0x00},
    {1, 0x00},
    {1, 0x00},
    {1, 0x00},
    {1, 0x11},
    {1, 0x00},
    {4, 0x00, 0x00, 0x00, 0x00},
    {1, 0x00},
    {1, 0x00},
    {1, 0x00},
    {1, 0x00},
    {4, 0x01, 0x02, 0x03, 0x04},
    {4, 0x01, 0x02, 0x03, 0x04},
    {4, 0x01, 0x02, 0x03, 0x04},
    {8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {8, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
     0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
     0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {16, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
     0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {5, 0x01, 0x02, 0x03, 0x04, 0x05},
    {5, 0x01, 0x02, 0x03, 0x04, 0x05},
    {5, 0x01, 0x02, 0x03, 0x04, 0x05},
    {5, 0x01, 0x02, 0x03, 0x04, 0x05},
    {5, 0x01, 0x02, 0x03, 0x04, 0x05},
};

static unsigned char rc5_cbc_plain[RC5_CBC_NUM][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x01},
};

static int rc5_cbc_rounds[RC5_CBC_NUM] = {
    0, 0, 0, 0, 0, 1, 2, 2,
    8, 8, 12, 16, 8, 12, 16, 12,
    8, 12, 16, 8, 12, 16, 12, 8,
    8, 8, 8,
};

static unsigned char rc5_cbc_iv[RC5_CBC_NUM][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x78, 0x75, 0xdb, 0xf6, 0x73, 0x8c, 0x64, 0x78},
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x7c, 0xb3, 0xf1, 0xdf, 0x34, 0xf9, 0x48, 0x11},
};

static int test_rc5_ecb(int n)
{
    int testresult = 1;
    RC5_32_KEY key;
    unsigned char buf[8], buf2[8];

    if (!TEST_true(RC5_32_set_key(&key, 16, &RC5key[n][0], 12)))
        return 0;

    RC5_32_ecb_encrypt(&RC5plain[n][0], buf, &key, RC5_ENCRYPT);
    if (!TEST_mem_eq(&RC5cipher[n][0], sizeof(RC5cipher[0]), buf, sizeof(buf)))
        testresult = 0;

    RC5_32_ecb_encrypt(buf, buf2, &key, RC5_DECRYPT);
    if (!TEST_mem_eq(&RC5plain[n][0], sizeof(RC5cipher[0]), buf2, sizeof(buf2)))
        testresult = 0;

    return testresult;
}

static int test_rc5_cbc(int n)
{
    int testresult = 1;
    int i;
    RC5_32_KEY key;
    unsigned char buf[8], buf2[8], ivb[8];

    i = rc5_cbc_rounds[n];
    if (i >= 8) {
        if (!TEST_true(RC5_32_set_key(&key, rc5_cbc_key[n][0],
                                      &rc5_cbc_key[n][1], i)))
            return 0;

        memcpy(ivb, &rc5_cbc_iv[n][0], 8);
        RC5_32_cbc_encrypt(&rc5_cbc_plain[n][0], buf, 8,
                           &key, &ivb[0], RC5_ENCRYPT);

        if (!TEST_mem_eq(&rc5_cbc_cipher[n][0], sizeof(rc5_cbc_cipher[0]),
                         buf, sizeof(buf)))
            testresult = 0;

        memcpy(ivb, &rc5_cbc_iv[n][0], 8);
        RC5_32_cbc_encrypt(buf, buf2, 8, &key, &ivb[0], RC5_DECRYPT);
        if (!TEST_mem_eq(&rc5_cbc_plain[n][0], sizeof(rc5_cbc_plain[0]),
                         buf2, sizeof(buf2)))
            testresult = 0;
    }

    return testresult;
}
#endif

int setup_tests(void)
{
#ifndef OPENSSL_NO_RC5
    ADD_ALL_TESTS(test_rc5_ecb, OSSL_NELEM(RC5key));
    ADD_ALL_TESTS(test_rc5_cbc, RC5_CBC_NUM);
#endif
    return 1;
}
