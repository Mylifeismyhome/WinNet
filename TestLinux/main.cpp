#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>

int main()
{
	Net::String str("Test");
	str.append(" ES FUNKT!");
	printf(str.get().get());
	return 0;
}
