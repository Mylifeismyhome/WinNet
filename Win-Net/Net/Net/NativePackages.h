#pragma once
#include <Net/Net.h>
#include <Net/Package.h>

#define NET_NATIVE_PACKAGE_ID Net::NativePackages::PacketTypes
#define NET_LAST_PACKAGE_ID Net::NativePackages::PacketTypes::PKG_LAST_PACKAGE

NET_DSA_BEGIN

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(NativePackages)
enum PacketTypes
{
	PKG_RSAHandshake = 0,
	PKG_VersionPackage,
	PKG_EstabilishPackage,
	PKG_ClosePackage,

	PKG_LAST_PACKAGE
};
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END