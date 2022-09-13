#pragma once
#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>

#define NET_SHA1 Net::Coding::SHA1
#define NET_SHA1_HEX_SIZE (40 + 1)
#define NET_SHA1_BASE64_SIZE (28 + 1)

NET_DSA_BEGIN
namespace Net
{
	namespace Coding
	{
		class SHA1
		{
			uint32_t state[5];
			uint8_t buf[64];
			uint32_t i;
			uint64_t n_bits;

			void add_byte_dont_count_bits(uint8_t);
			void process_block(const uint8_t*);

		public:
			SHA1(const char* = NULL);

			SHA1& add(uint8_t);
			SHA1& add(char);

			SHA1& add(const void*, uint32_t);
			SHA1& add(const char*);
			SHA1& finalize();

			const SHA1& to_hex(char*, bool = true, const char* = CSTRING("0123456789abcdef"));
			const SHA1& to_base64(char*, bool = true);
		};
	}
}
NET_DSA_END