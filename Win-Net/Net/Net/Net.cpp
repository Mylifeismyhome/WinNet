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

const char* Net::KEY()
{
	return NetKey;
}

const char* Net::VERSION()
{
	static char version[MAX_PATH];
	sprintf_s(version, CSTRING("%i.%i.%i-%s"),		MAJOR_VERSION(), MINOR_VERSION(), REVISION(), KEY());
	return version;
}