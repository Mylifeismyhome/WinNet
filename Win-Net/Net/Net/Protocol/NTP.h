#pragma once

#define NTP_TIMESTAMP_DELTA 2208988800ull
#define NTP_DEFAULT_PORT 123

#include <Net/Net/Net.h>
#include <Net/assets/manager/logmanager.h>

#ifdef BUILD_LINUX
#define LAST_ERROR errno
#define NTP_SOCKET_LEN socklen_t
#else
#define LAST_ERROR Ws2_32::WSAGetLastError()
#define NTP_SOCKET_LEN int
#endif

NET_DSA_BEGIN
namespace Net
{
	namespace Protocol
	{
		namespace NTP
		{
			struct NTP_FRAME
			{
				uint8_t li_vn_mode; // Eight bits. li, vn, and mode.
				// li.   Two bits.   Leap indicator.
				// vn.   Three bits. Version number of the protocol.
				// mode. Three bits. Client will pick mode 3 for client.

				uint8_t stratum;         // Eight bits. Stratum level of the local clock.
				uint8_t poll;            // Eight bits. Maximum interval between successive messages.
				uint8_t precision;       // Eight bits. Precision of the local clock.

				uint32_t rootDelay;      // 32 bits. Total round trip delay time.
				uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
				uint32_t refId;          // 32 bits. Reference clock identifier.

				uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
				uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

				uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
				uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

				uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
				uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

				uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
				uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.
			}; // Total: 384 bits or 48 bytes.

			class NTPRes
			{
				NTP_FRAME nframe;
				bool bvalid;

			public:
				NTPRes();
				NTPRes(NTP_FRAME&);

				NTP_FRAME& frame();
				bool valid() const;
			};

			char* ResolveHostname(const char*);
			NTPRes Perform(const char*, u_short = NTP_DEFAULT_PORT);
			NTPRes Exec(const char*, u_short = NTP_DEFAULT_PORT);
			NTPRes Run(const char*, u_short = NTP_DEFAULT_PORT);
		}
	}
}
NET_DSA_END