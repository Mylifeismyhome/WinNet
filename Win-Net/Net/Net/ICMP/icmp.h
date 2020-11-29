#pragma once
#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>

NET_DSA_BEGIN

// Sample: Protocol header definitions used by ping (raw sockets)

//

// Files:

//      iphdrv2.h     - this file

//

// Description:

//      This file contains various protocol header definitions used by

//      the raw socket ping sample.

//

// Align on a 1-byte boundary

#include <pshpack1.h>



// IPv4 header

typedef struct ip_hdr

{

	unsigned char  ip_verlen;          // 4-bit IPv4 version 4-bit header length (in 32-bit words)

	unsigned char  ip_tos;               // IP type of service

	unsigned short ip_totallength;   // Total length

	unsigned short ip_id;                 // Unique identifier

	unsigned short ip_offset;           // Fragment offset field

	unsigned char  ip_ttl;                  // Time to live

	unsigned char  ip_protocol;       // Protocol(TCP,UDP etc)

	unsigned short ip_checksum;    // IP checksum

	unsigned int   ip_srcaddr;           // Source address

	unsigned int   ip_destaddr;         // Source address

} IPV4_HDR, * PIPV4_HDR, FAR* LPIPV4_HDR;



// IPv4 option header

typedef struct ipv4_option_hdr

{

	unsigned char   opt_code;        // option type

	unsigned char   opt_len;            // length of the option header

	unsigned char   opt_ptr;            // offset into options

	unsigned long   opt_addr[9];     // list of IPv4 addresses

} IPV4_OPTION_HDR, * PIPV4_OPTION_HDR, FAR* LPIPV4_OPTION_HDR;



// ICMP header

typedef struct icmp_hdr

{

	unsigned char   icmp_type;

	unsigned char   icmp_code;

	unsigned short  icmp_checksum;

	unsigned short  icmp_id;

	unsigned short  icmp_sequence;

	unsigned long   icmp_timestamp;

} ICMP_HDR, * PICMP_HDR, FAR* LPICMP_HDR;



// IPv6 protocol header

typedef struct ipv6_hdr

{

	unsigned long   ipv6_vertcflow;        // 4-bit IPv6 version, 8-bit traffic class, 20-bit flow label

	unsigned short  ipv6_payloadlen;    // payload length

	unsigned char   ipv6_nexthdr;          // next header protocol value

	unsigned char   ipv6_hoplimit;         // TTL

	struct in6_addr ipv6_srcaddr;          // Source address

	struct in6_addr ipv6_destaddr;         // Destination address

} IPV6_HDR, * PIPV6_HDR, FAR* LPIPV6_HDR;



// IPv6 fragment header

typedef struct ipv6_fragment_hdr

{

	unsigned char   ipv6_frag_nexthdr;

	unsigned char   ipv6_frag_reserved;

	unsigned short  ipv6_frag_offset;

	unsigned long   ipv6_frag_id;

} IPV6_FRAGMENT_HDR, * PIPV6_FRAGMENT_HDR, FAR* LPIPV6_FRAGMENT_HDR;



// ICMPv6 header

typedef struct icmpv6_hdr {

	unsigned char   icmp6_type;

	unsigned char   icmp6_code;

	unsigned short  icmp6_checksum;

} ICMPV6_HDR;



// ICMPv6 echo request body

typedef struct icmpv6_echo_request

{

	unsigned short  icmp6_echo_id;

	unsigned short  icmp6_echo_sequence;

} ICMPV6_ECHO_REQUEST;



// Define the UDP header

typedef struct udp_hdr

{

	unsigned short src_portno;       // Source port no.

	unsigned short dst_portno;       // Dest. port no.

	unsigned short udp_length;       // Udp packet length

	unsigned short udp_checksum;     // Udp checksum (optional)

} UDP_HDR, * PUDP_HDR;



// IPv4 option for record route

#define IP_RECORD_ROUTE     0x7



// ICMP6 protocol value (used in the socket call and IPv6 header)

#define IPPROTO_ICMP6       58



// ICMP types and codes

#define ICMPV4_ECHO_REQUEST_TYPE   8

#define ICMPV4_ECHO_REQUEST_CODE   0

#define ICMPV4_ECHO_REPLY_TYPE     0

#define ICMPV4_ECHO_REPLY_CODE     0

#define ICMPV4_MINIMUM_HEADER      8



// ICPM6 types and codes

#define ICMPV6_ECHO_REQUEST_TYPE   128

#define ICMPV6_ECHO_REQUEST_CODE   0

#define ICMPV6_ECHO_REPLY_TYPE     129

#define ICMPV6_ECHO_REPLY_CODE     0



// Restore byte alignment back to default

#include <poppack.h>

 // Common routines for resolving addresses and hostnames

// Files:

//      resolve.h       - Header file for common routines

// Description:

//      This file contains common name resolution and name printing

//      routines and is used by many of the samples on this CD.

//

#ifndef _RESOLVE_H_

#define _RESOLVE_H_



#ifdef _cplusplus

extern "C" {

#endif



	int              PrintAddress(SOCKADDR* sa, int salen);

	int              FormatAddress(SOCKADDR* sa, int salen, char* addrbuf, int addrbuflen);

	int              ReverseLookup(SOCKADDR* sa, int salen, char* namebuf, int namebuflen);

	struct addrinfo* ResolveAddress(char* addr, char* port, int af, int type, int proto);



#ifdef _cplusplus

}

#endif

#ifdef _WIN64
typedef DWORD64 lt;
#else
typedef DWORD lt;
#endif

#define DEFAULT_DATA_SIZE      32               // default data size
#define DEFAULT_SEND_COUNT     4            // number of ICMP requests to send
#define DEFAULT_RECV_TIMEOUT   6000    // six second
#define DEFAULT_TTL            128

NET_CLASS_BEGIN(ICMP)
SOCKET _socket;
lt _latency;
char _address[255];
int gAddressFamily;// Address family to use
int   gDataSize; // Amount of data to send
BOOL  bRecordRoute; // Use IPv4 record route?
char* gDestination;// Destination

NET_CLASS_PUBLIC
ICMP(const char*);

void execute();
lt getLatency() const;
NET_CLASS_END

#endif

NET_DSA_END