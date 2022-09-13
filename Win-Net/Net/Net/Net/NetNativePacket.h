#pragma once

#define NET_NATIVE_PACKAGE_ID Net::NativePacket::PacketTypes
#define NET_LAST_PACKAGE_ID Net::NativePacket::PacketTypes::PKG_LAST_PACKAGE

#include <Net/Net/Net.h>
#include <Net/Net/NetPacket.h>

NET_DSA_BEGIN
namespace Net
{
	namespace NativePacket
	{
		enum PacketTypes
		{
			PKG_RSAHandshake = 0,
			PKG_VersionPackage,
			PKG_EstabilishPackage,
			PKG_ClosePackage,

			PKG_LAST_PACKAGE
		};
	}
}
NET_DSA_END