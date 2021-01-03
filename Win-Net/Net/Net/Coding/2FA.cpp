#include "2FA.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
/** The type of a single byte. */
typedef uint8_t Byte;

/** The type of a byte string. */
typedef std::basic_string<Byte> ByteString;

static void clearByteString(ByteString* bstr)
{
	volatile Byte* bs = const_cast<volatile Byte*>(bstr->data());

	for (size_t i = 0; i < bstr->size(); ++i)
	{
		bs[i] = Byte(0);
	}
}

static void swizzleByteStrings(ByteString* target, ByteString* source)
{
	clearByteString(target);
	target->assign(*source);
	clearByteString(source);
}

static char nibbleToLCHex(uint8_t nib)
{
	if (nib < 0xa)
	{
		return static_cast<char>(nib + '0');
	}
	else if (nib < 0x10)
	{
		return static_cast<char>((nib - 10) + 'a');
	}
	else
	{
		assert(0 && CSTRING("not actually a nibble"));
		return '\0';
	}
}

static uint8_t hexToNibble(char c)
{
	if (c >= '0' && c <= '9')
	{
		return static_cast<uint8_t>(c - '0');
	}
	else if (c >= 'A' && c <= 'F')
	{
		return static_cast<uint8_t>(c - 'A' + 10);
	}
	else if (c >= 'a' && c <= 'f')
	{
		return static_cast<uint8_t>(c - 'a' + 10);
	}
	else
	{
		assert(0 && CSTRING("not actually a hex digit"));
		return 0xff;
	}
}

static std::string toHexString(const ByteString& bstr)
{
	std::string ret;

	for (Byte b : bstr)
	{
		ret.push_back(nibbleToLCHex((b >> 4) & 0x0F));
		ret.push_back(nibbleToLCHex((b >> 0) & 0x0F));
	}

	return ret;
}

static ByteString fromHexStringSkipUnknown(const std::string& str)
{
	std::string hstr;
	for (char c : str)
	{
		if (
			(c >= '0' && c <= '9') ||
			(c >= 'A' && c <= 'F') ||
			(c >= 'a' && c <= 'f')
			)
		{
			hstr.push_back(c);
		}
		// ignore otherwise
	}

	if (hstr.size() % 2 != 0)
	{
		throw std::invalid_argument(CSTRING("hex string (unknown characters ignored) length not divisible by 2"));
	}

	ByteString ret;
	for (size_t i = 0; i < hstr.size(); i += 2)
	{
		uint8_t top = hexToNibble(hstr[i + 0]);
		uint8_t btm = hexToNibble(hstr[i + 1]);

		ret.push_back((top << 4) | btm);
	}
	return ret;
}

static	ByteString u32beToByteString(uint32_t num)
{
	ByteString ret;
	ret.push_back((num >> 24) & 0xFF);
	ret.push_back((num >> 16) & 0xFF);
	ret.push_back((num >> 8) & 0xFF);
	ret.push_back((num >> 0) & 0xFF);
	return ret;
}

static ByteString u64beToByteString(uint64_t num)
{
	ByteString left = u32beToByteString((num >> 32) & 0xFFFFFFFF);
	ByteString right = u32beToByteString((num >> 0) & 0xFFFFFFFF);
	return left + right;
}

static ByteString b32ChunkToBytes(const std::string& str)
{
	ByteString ret;
	uint64_t whole = 0x00;
	size_t padcount = 0;
	size_t finalcount;

	if (str.length() != 8)
	{
		throw std::invalid_argument(CSTRING("incorrect length of base32 chunk"));
	}

	size_t i;

	for (i = 0; i < 8; ++i)
	{
		char c = str[i];
		uint64_t bits;

		if (c == '=')
		{
			bits = 0;
			++padcount;
		}
		else if (padcount > 0)
		{
			throw std::invalid_argument(CSTRING("padding character followed by non-padding character"));
		}
		else if (c >= 'A' && c <= 'Z')
		{
			bits = static_cast<Byte>(c - 'A');
		}
		else if (c >= '2' && c <= '7')
		{
			bits = static_cast<Byte>(c - '2' + 26);
		}
		else
		{
			throw std::invalid_argument(CSTRING("not a base32 character: ") + std::string(1, c));
		}

		// shift into the chunk
		whole |= (bits << ((7 - i) * 5));
	}

	switch (padcount)
	{
	case 0:
		finalcount = 5;
		break;
	case 1:
		finalcount = 4;
		break;
	case 3:
		finalcount = 3;
		break;
	case 4:
		finalcount = 2;
		break;
	case 6:
		finalcount = 1;
		break;
	default:
		throw std::invalid_argument(CSTRING("invalid number of padding characters"));
	}

	for (i = 0; i < finalcount; ++i)
	{
		// shift out of the chunk
		ret.push_back(static_cast<Byte>((whole >> ((4 - i) * 8)) & 0xFF));
	}

	return ret;
}

static inline uint64_t u64(uint8_t n)
{
	return static_cast<uint64_t>(n);
}

static std::string bytesToB32Chunk(const ByteString& bs)
{
	if (bs.size() < 1 || bs.size() > 5)
	{
		throw std::invalid_argument(CSTRING("need a chunk of at least 1 and at most 5 bytes"));
	}

	uint64_t whole = 0x00;
	size_t putchars = 2;
	std::string ret;

	// shift into the chunk
	whole |= (u64(bs[0]) << 32);
	if (bs.size() > 1)
	{
		whole |= (u64(bs[1]) << 24);
		putchars += 2;  // at least 4
	}
	if (bs.size() > 2)
	{
		whole |= (u64(bs[2]) << 16);
		++putchars;  // at least 5
	}
	if (bs.size() > 3)
	{
		whole |= (u64(bs[3]) << 8);
		putchars += 2;  // at least 7
	}
	if (bs.size() > 4)
	{
		whole |= u64(bs[4]);
		++putchars;  // at least 8
	}

	size_t i;
	for (i = 0; i < putchars; ++i)
	{
		// shift out of the chunk

		Byte val = (whole >> ((7 - i) * 5)) & 0x1F;

		// map bits to base32

		if (val < 26)
		{
			ret.push_back(static_cast<char>(val + 'A'));
		}
		else
		{
			ret.push_back(static_cast<char>(val - 26 + '2'));
		}
	}

	// pad

	for (i = putchars; i < 8; ++i)
	{
		ret.push_back('=');
	}

	return ret;
}

/** Deletes the contets of a byte string on destruction. */
class ByteStringDestructor
{
private:
	/** The byte string to clear. */
	ByteString* m_bs;

public:
	ByteStringDestructor(ByteString* bs) : m_bs(bs) {}
	~ByteStringDestructor() { clearByteString(m_bs); }
};

typedef ByteString(*HmacFunc)(const ByteString&, const ByteString&);

static inline uint32_t lrot32(uint32_t num, uint8_t rotcount)
{
	return (num << rotcount) | (num >> (32 - rotcount));
}

static ByteString sha1(const ByteString& msg)
{
	const size_t size_bytes = msg.size();
	const uint64_t size_bits = size_bytes * 8;
	ByteString bstr = msg;
	ByteStringDestructor asplode(&bstr);

	// the size of msg in bits is always even. adding the '1' bit will make
	// it odd and therefore incongruent to 448 modulo 512, so we can get
	// away with tacking on 0x80 and then the 0x00s.
	bstr.push_back(0x80);
	while (bstr.size() % (512 / 8) != (448 / 8))
	{
		bstr.push_back(0x00);
	}

	// append the size in bits (uint64be)
	bstr.append(u64beToByteString(size_bits));

	assert(bstr.size() % (512 / 8) == 0);

	// initialize the hash counters
	uint32_t h0 = 0x67452301;
	uint32_t h1 = 0xEFCDAB89;
	uint32_t h2 = 0x98BADCFE;
	uint32_t h3 = 0x10325476;
	uint32_t h4 = 0xC3D2E1F0;

	// for each 64-byte chunk
	for (size_t i = 0; i < bstr.size() / 64; ++i)
	{
		ByteString chunk(bstr.begin() + i * 64, bstr.begin() + (i + 1) * 64);
		ByteStringDestructor xplode(&chunk);

		uint32_t words[80];
		size_t j;

		// 0-15: the chunk as a sequence of 32-bit big-endian integers
		for (j = 0; j < 16; ++j)
		{
			words[j] =
				(chunk[4 * j + 0] << 24) |
				(chunk[4 * j + 1] << 16) |
				(chunk[4 * j + 2] << 8) |
				(chunk[4 * j + 3] << 0)
				;
		}

		// 16-79: derivatives of 0-15
		for (j = 16; j < 32; ++j)
		{
			// unoptimized
			words[j] = lrot32(words[j - 3] ^ words[j - 8] ^ words[j - 14] ^ words[j - 16], 1);
		}
		for (j = 32; j < 80; ++j)
		{
			// Max Locktyuchin's optimization (SIMD)
			words[j] = lrot32(words[j - 6] ^ words[j - 16] ^ words[j - 28] ^ words[j - 32], 2);
		}

		// initialize hash values for the round
		uint32_t a = h0;
		uint32_t b = h1;
		uint32_t c = h2;
		uint32_t d = h3;
		uint32_t e = h4;

		// the loop
		for (j = 0; j < 80; ++j)
		{
			uint32_t f = 0, k = 0;

			if (j < 20)
			{
				f = (b & c) | ((~b) & d);
				k = 0x5A827999;
			}
			else if (j < 40)
			{
				f = b ^ c ^ d;
				k = 0x6ED9EBA1;
			}
			else if (j < 60)
			{
				f = (b & c) | (b & d) | (c & d);
				k = 0x8F1BBCDC;
			}
			else if (j < 80)
			{
				f = b ^ c ^ d;
				k = 0xCA62C1D6;
			}
			else
			{
				assert(0 && CSTRING("internal error"));
			}

			uint32_t tmp = lrot32(a, 5) + f + e + k + words[j];
			e = d;
			d = c;
			c = lrot32(b, 30);
			b = a;
			a = tmp;
		}

		// add that to the result so far
		h0 += a;
		h1 += b;
		h2 += c;
		h3 += d;
		h4 += e;
	}

	// assemble the digest
	ByteString first = u32beToByteString(h0);
	ByteStringDestructor x1(&first);
	ByteString second = u32beToByteString(h1);
	ByteStringDestructor x2(&second);
	ByteString third = u32beToByteString(h2);
	ByteStringDestructor x3(&third);
	ByteString fourth = u32beToByteString(h3);
	ByteStringDestructor x4(&fourth);
	ByteString fifth = u32beToByteString(h4);
	ByteStringDestructor x5(&fifth);

	return first + second + third + fourth + fifth;
}

static ByteString hmacSha1(const ByteString& key, const ByteString& msg, size_t blockSize = 64)
{
	ByteString realKey = key;
	ByteStringDestructor asplode(&realKey);

	if (realKey.size() > blockSize)
	{
		// resize by calculating hash
		ByteString newRealKey = sha1(realKey);
		swizzleByteStrings(&realKey, &newRealKey);
	}
	if (realKey.size() < blockSize)
	{
		// pad with zeroes
		realKey.resize(blockSize, 0x00);
	}

	// prepare the pad keys
	ByteString innerPadKey = realKey;
	ByteStringDestructor xplodeI(&innerPadKey);
	ByteString outerPadKey = realKey;
	ByteStringDestructor xplodeO(&outerPadKey);

	// transform the pad keys
	for (size_t i = 0; i < realKey.size(); ++i)
	{
		innerPadKey[i] = innerPadKey[i] ^ 0x36;
		outerPadKey[i] = outerPadKey[i] ^ 0x5c;
	}

	// sha1(outerPadKey + sha1(innerPadKey + msg))
	ByteString innerMsg = innerPadKey + msg;
	ByteStringDestructor xplodeIM(&innerMsg);
	ByteString innerHash = sha1(innerMsg);
	ByteStringDestructor xplodeIH(&innerHash);
	ByteString outerMsg = outerPadKey + innerHash;
	ByteStringDestructor xplodeOM(&outerMsg);

	return sha1(outerMsg);
}

/*
* The 64-bit-blocksize variant of HMAC-SHA1.
*/
static ByteString hmacSha1_64(const ByteString& key, const ByteString& msg) {
	return hmacSha1(key, msg, 64);
}

/*
 * Calculate the HOTP value of the given key, message and digit count.
 */
static uint32_t hotp(const ByteString& key, uint64_t counter, size_t digitCount = 6, HmacFunc hmacf = hmacSha1_64)
{
	ByteString msg = u64beToByteString(counter);
	ByteStringDestructor dmsg(&msg);

	ByteString hmac = hmacf(key, msg);
	ByteStringDestructor dhmac(&hmac);

	uint32_t digits10 = 1;
	for (size_t i = 0; i < digitCount; ++i)
	{
		digits10 *= 10;
	}

	// fetch the offset (from the last nibble)
	uint8_t offset = hmac[hmac.size() - 1] & 0x0F;

	// fetch the four bytes from the offset
	ByteString fourWord = hmac.substr(offset, 4);
	ByteStringDestructor dfourWord(&fourWord);

	// turn them into a 32-bit integer
	uint32_t ret =
		(fourWord[0] << 24) |
		(fourWord[1] << 16) |
		(fourWord[2] << 8) |
		(fourWord[3] << 0)
		;

	// snip off the MSB (to alleviate signed/unsigned troubles)
	// and calculate modulo digit count
	return (ret & 0x7fffffff) % digits10;
}

/*
 * Calculate the TOTP value from the given parameters.
 */
static uint32_t totp(const ByteString& key, uint64_t timeNow, uint64_t timeStart, uint64_t timeStep, size_t digitCount = 6, HmacFunc hmacf = hmacSha1_64)
{
	uint64_t timeValue = (timeNow - timeStart) / timeStep;
	return hotp(key, timeValue, digitCount, hmacf);
}

uint32_t FA2::generateToken(const byte* t_secret, const size_t len, time_t time, const int& t_interval)
{
	return totp(ByteString(t_secret, t_secret + len), time, 0, t_interval, 6);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
