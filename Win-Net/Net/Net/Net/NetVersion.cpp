#include <Net/Net/NetVersion.h>

short Net::Version::Major()
{
	return NetMajorVersion;
}

short Net::Version::Minor()
{
	return NetMinorVersion;
}

short Net::Version::Revision()
{
	return NetRevision;
}

Net::String Net::Version::Key()
{
	return Net::String(NetKey);
}

Net::String Net::Version::Version()
{
	return Net::String(CSTRING("%i.%i.%i-%s"), Major(), Minor(), Revision(), Key().data().data());
}