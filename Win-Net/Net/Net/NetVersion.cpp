#include <Net/NetVersion.h>

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

NetString Net::Version::Key()
{
	return NetString(NetKey);
}

NetString Net::Version::Version()
{
	return NetString(CSTRING("%i.%i.%i-%s"), Major(), Minor(), Revision(), Key().data().data());
}