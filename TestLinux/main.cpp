#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Cryption/AES.h>

int main()
{
	Net::String str("Test");
	str.append(" ES FUNKT!");
	printf(str.get().get());

	NET_AES aes;
	aes.init("TEST", "HUAN");

	BYTE* enc = ALLOC<BYTE>(strlen("TEST") + 1);
	memcpy(enc, "TEST", strlen("TEST");
	enc[strlen("TEST")] = '\0';

	aes.encrypt(enc);

	printf(enc);

	FREE(enc);
	return 0;
}
