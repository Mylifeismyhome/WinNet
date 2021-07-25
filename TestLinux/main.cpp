#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Cryption/AES.h>

int main()
{
	Net::String str("Test");
	str.append(" ES FUNKT!");
	printf(str.get().get());

	NET_AES aes;
	aes.Init((char*)"TEST", (char*)"HUAN");

	const char* word = "test du huan";
	size_t size = strlen(word);
	BYTE* enc = ALLOC<BYTE>(size + 1);
	memcpy(enc, word, size);
	enc[size] = '\0';

	aes.encrypt(enc, size);

	printf("\n%s", enc);

	aes.decrypt(enc, size);

	printf("\n\n%s", enc);

	FREE(enc);
	return 0;
}
