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

	BYTE* enc = ALLOC<BYTE>(strlen("TEST") + 1);
	memcpy(enc, "TEST", strlen("TEST"));
	enc[strlen("TEST")] = '\0';

	size_t size = strlen("TEST");
	aes.encrypt(enc, size);

	printf("%s", enc);

	FREE(enc);
	return 0;
}
