#pragma once
#define NET_NATIVE_PACKAGE_ID Net::NativePackages::PacketTypes
#define NET_LAST_PACKAGE_ID Net::NativePackages::PacketTypes::PKG_LAST_PACKAGE

#include <Net/Net/Net.h>
#include <Net/Net/NetPacket.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(NativePackages)
NET_DSA_BEGIN
enum PacketTypes
{
	PKG_RSAHandshake = 0,
	PKG_VersionPackage,
	PKG_EstabilishPackage,
	PKG_ClosePackage,

	PKG_LAST_PACKAGE
};
NET_DSA_END
NET_NAMESPACE_END
NET_NAMESPACE_END