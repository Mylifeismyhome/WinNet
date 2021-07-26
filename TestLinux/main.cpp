#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Coding/Hex.h>

int main()
{
	LOG_ERROR(CSTRING("TEST %i"), 50);


	const char* word = "Ich glaube es funktioniert!";
	size_t size = strlen(word);

	BYTE* data = ALLOC<BYTE>(size + 1);
	memcpy(data, word, size);
	data[size] = '\0';

	byte* out = nullptr;
	Net::Coding::Hex::encode(data, out, size);

	printf("%s", out);


	Net::Coding::Hex::decode(out, data, size);

	printf("\n%s", data);

	FREE(data);
	return 0;
}
