#pragma once
#include <Net/Net/Net.h>
#include <Net/Net/NetPacket.h>

namespace Sandbox
{
	NET_DEFINE_PACKETS(Packet,
		PKG_TEST,
		PKG_TEST_RAW
	);
}
