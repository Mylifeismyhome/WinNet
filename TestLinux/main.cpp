#define TEST(name, fnc) void name () { LOG(CSTRING("---------------------------------------")); LOG(CSTRING("Test Case: " #name)); LOG(CSTRING("----------------------------------------")); fnc LOG(CSTRING("----------------------------------------")); }
#define RUN(name) name ();

#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>

// coding
#include <Net/Coding/Hex.h>
#include <Net/Coding/BASE32.h>
#include <Net/Coding/BASE64.h>

TEST(Hex,
 	const char* word = "Ich versuche mich mal auf Hex";
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

	LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Hex::encode(data, out, size);

        LOG(CSTRING("Hex Encoded: %s"), out);

        Net::Coding::Hex::decode(out, data, size);

	LOG(CSTRING("Hex Decoded: %s"), data);

	FREE(data);
);

TEST(Base32,
	const char* word = "Ich versuche mich mal auf Base32";
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

        LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Base32::encode(data, out, size);

        LOG(CSTRING("Base32 Encoded: %s"), out);

        Net::Coding::Base32::decode(out, data, size);

        LOG(CSTRING("Base32 Decoded: %s"), data);

        FREE(data);
);

TEST(Base64,
        const char* word = "Ich versuche mich mal auf Base64";
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

        LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Base64::encode(data, out, size);

        LOG(CSTRING("Base64 Encoded: %s"), out);

        Net::Coding::Base64::decode(out, data, size);

        LOG(CSTRING("Base64 Decoded: %s"), data);

        FREE(data);
);

int main()
{
	RUN(Hex);
	RUN(Base32);
	RUN(Base64);

	return 0;
}
