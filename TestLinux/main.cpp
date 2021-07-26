#define TEST(name, fnc) void name () { fnc }
#define RUN(name) name ();

#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Coding/Hex.h>

TEST(Hex,
 	const char* word = "Ich glaube es funktioniert!";
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

int main()
{
	RUN(Hex);
	return 0;
}
