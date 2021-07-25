#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Cryption/AES.h>
#include <Net/Coding/BASE64.h>

int main()
{
	LOG_ERROR(CSTRING("TEST %i"), 50);


	const char* word = "test du huan";
	size_t size = strlen(word);

	BYTE* data = ALLOC<BYTE>(size + 1);
	memcpy(data, word, size);
	data[size] = '\0';

	Net::Coding::Base64::encode(data, size);

	printf("%s", data);


	Net::Coding::Base64::decode(data, size);

	printf("\n%s", data);

	FREE(data);
	return 0;
	// not used

	Net::String str("Test");
	str.append(" ES FUNKT!");
	printf(str.get().get());

	NET_AES aes;
	aes.Init((char*)"TEST", (char*)"HUAN");

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
