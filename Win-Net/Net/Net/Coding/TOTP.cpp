#include "TOTP.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
/** The type of a byte string. */
typedef std::basic_string<uint8_t> ByteString;

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

void hmac_sha1(const uint8_t* k,  /* secret key */
	size_t lk,       /* length of the key in bytes */
	const uint8_t* d,  /* data */
	size_t ld,       /* length of data in bytes */
	uint8_t* out,      /* output buffer, at least "t" bytes */
	size_t* t)
{
	SHA_CTX ictx, octx;
	uint8_t isha[SHA_DIGEST_LENGTH], osha[SHA_DIGEST_LENGTH];
	uint8_t key[SHA_DIGEST_LENGTH];
	uint8_t buf[SHA_BLOCKSIZE];

	if (lk > SHA_BLOCKSIZE)
	{
		SHA_CTX tctx;

		SHA1_Init(&tctx);
		SHA1_Update(&tctx, k, lk);
		SHA1_Final(key, &tctx);

		k = key;
		lk = SHA_DIGEST_LENGTH;
	}

	/**** Inner Digest ****/

	SHA1_Init(&ictx);

	/* Pad the key for inner digest */
	for (size_t i = 0; i < lk; ++i)
		buf[i] = k[i] ^ 0x36;

	for (size_t i = lk; i < SHA_BLOCKSIZE; ++i)
		buf[i] = 0x36;

	SHA1_Update(&ictx, buf, SHA_BLOCKSIZE);
	SHA1_Update(&ictx, d, ld);

	SHA1_Final(isha, &ictx);

	/**** Outer Digest ****/

	SHA1_Init(&octx);

	/* Pad the key for outter digest */

	for (size_t i = 0; i < lk; ++i)
		buf[i] = k[i] ^ 0x5c;

	for (size_t i = lk; i < SHA_BLOCKSIZE; ++i)
		buf[i] = 0x5c;

	SHA1_Update(&octx, buf, SHA_BLOCKSIZE);
	SHA1_Update(&octx, isha, SHA_DIGEST_LENGTH);

	SHA1_Final(osha, &octx);

	/* truncate and print the results */
	*t = *t > SHA_DIGEST_LENGTH ? SHA_DIGEST_LENGTH : *t;
	memcpy(out, osha, *t);
}

/*
 * Calculate the HOTP value of the given key, message and digit count.
 */
uint32_t hotp(const ByteString& key, uint64_t counter, size_t digitCount = 6)
{
	ByteString msg = u64beToByteString(counter);

	byte bhmac[256] = { 0 };
	size_t len = sizeof(bhmac);

	hmac_sha1(key.c_str(), key.length(), msg.data(), msg.length(), bhmac, &len);

	ByteString hmac = { bhmac , bhmac + len };

	uint32_t digits10 = 1;
	for (size_t i = 0; i < digitCount; ++i)
		digits10 *= 10;

	// fetch the offset (from the last nibble)
	uint8_t offset = hmac[hmac.size() - 1] & 0x0F;

	// fetch the four bytes from the offset
	ByteString fourWord = hmac.substr(offset, 4);

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
uint32_t totp(const ByteString& key, uint64_t timeNow, uint64_t timeStart, uint64_t timeStep, size_t digitCount = 6)
{
	uint64_t timeValue = (timeNow - timeStart) / timeStep;
	return hotp(key, timeValue, digitCount);
}

uint32_t TOTP::generateToken(const byte* t_secret, const size_t len, time_t time, const int t_interval)
{
	return totp(ByteString(t_secret, t_secret + len), time, 0, t_interval, 10);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
