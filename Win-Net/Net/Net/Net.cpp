#include <Net/Net.h>
#include "Cryption/XOR.h"

short Net::MAJOR_VERSION()
{
	return NetMajorVersion;
}

short Net::MINOR_VERSION()
{
	return NetMinorVersion;
}

short Net::REVISION()
{
	return NetRevision;
}

std::string Net::KEY()
{
	return std::string(NetKey);
}

std::string Net::VERSION()
{
	char version[MAX_PATH];
	sprintf_s(version, CSTRING("%i.%i.%i-%s"),	 MAJOR_VERSION(), MINOR_VERSION(), REVISION(), KEY());
	return std::string(version);
}

char* Net::ssl::GET_SSL_METHOD_NAME(const int method)
{
	for (auto& val : NET_SSL_METHOD_L)
	{
		if (val.method == method)
			return val.name;
	}

	return (char*)CSTRING("UNKNOWN");
}