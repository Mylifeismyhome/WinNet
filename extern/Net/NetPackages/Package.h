#pragma once
#include <Net/Package.h>

#define WebPackageID Net::Web_Server_Packages::PacketTypes

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Web_Server_Packages)
enum PacketTypes
{
	PKG_Test = 0,

	PKG_LAST_WEB_PACKAGES
};
END_NAMESPACE
END_NAMESPACE