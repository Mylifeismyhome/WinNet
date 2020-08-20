#pragma once
#include <Net/Net.h>
#include <Net/Package.h>

#define PackageID Net::NativePackages::PacketTypes

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(NativePackages)
enum PacketTypes
{
	PKG_RSAHandshake = 0,
	PKG_VersionPackage,
	PKG_EstabilishPackage,
	PKG_ClosePackage,

	PKG_LAST_PACKAGE
};
END_NAMESPACE
END_NAMESPACE