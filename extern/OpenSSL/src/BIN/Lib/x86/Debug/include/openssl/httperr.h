/*
 * Generated by util/mkerr.pl DO NOT EDIT
 * Copyright 1995-2019 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef OPENSSL_HTTPERR_H
# define OPENSSL_HTTPERR_H

# include <openssl/opensslconf.h>
# include <openssl/symhacks.h>


# ifdef  __cplusplus
extern "C"
# endif
int ERR_load_HTTP_strings(void);

/*
 * HTTP function codes.
 */
# ifndef OPENSSL_NO_DEPRECATED_3_0
# endif

/*
 * HTTP reason codes.
 */
# define HTTP_R_ASN1_LEN_EXCEEDS_MAX_RESP_LEN             108
# define HTTP_R_CONNECT_FAILURE                           100
# define HTTP_R_ERROR_PARSING_ASN1_LENGTH                 109
# define HTTP_R_ERROR_PARSING_CONTENT_LENGTH              119
# define HTTP_R_ERROR_PARSING_URL                         101
# define HTTP_R_ERROR_RECEIVING                           103
# define HTTP_R_ERROR_SENDING                             102
# define HTTP_R_INCONSISTENT_CONTENT_LENGTH               120
# define HTTP_R_MAX_RESP_LEN_EXCEEDED                     117
# define HTTP_R_MISSING_ASN1_ENCODING                     110
# define HTTP_R_MISSING_CONTENT_TYPE                      121
# define HTTP_R_MISSING_REDIRECT_LOCATION                 111
# define HTTP_R_REDIRECTION_FROM_HTTPS_TO_HTTP            112
# define HTTP_R_REDIRECTION_NOT_ENABLED                   116
# define HTTP_R_RESPONSE_LINE_TOO_LONG                    113
# define HTTP_R_SERVER_RESPONSE_PARSE_ERROR               104
# define HTTP_R_SERVER_SENT_ERROR                         105
# define HTTP_R_SERVER_SENT_WRONG_HTTP_VERSION            106
# define HTTP_R_STATUS_CODE_UNSUPPORTED                   114
# define HTTP_R_TLS_NOT_ENABLED                           107
# define HTTP_R_TOO_MANY_REDIRECTIONS                     115
# define HTTP_R_UNEXPECTED_CONTENT_TYPE                   118

#endif
