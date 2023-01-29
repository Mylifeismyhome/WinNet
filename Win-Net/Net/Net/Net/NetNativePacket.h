/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#pragma once

#define NET_NATIVE_PACKET_ID Net::NativePacket::PacketTypes
#define NET_LAST_PACKET_ID Net::NativePacket::PacketTypes::PKG_LAST_PACKET

#include <Net/Net/Net.h>
#include <Net/Net/NetPacket.h>

NET_DSA_BEGIN
namespace Net
{
	namespace NativePacket
	{
		enum PacketTypes
		{
			/* Init Server-Client Protocol option */
			PKG_NetProtocolHandshake = 0,

			/* Init Asymmetric Encryption */
			PKG_NetAsymmetricHandshake,

			PKG_NetEstabilish,
			PKG_NetClose,

			/* Net Hearbeat Packet to handle unused open sockets */
			PKG_NetHeartbeat,

			PKG_LAST_PACKET
		};
	}
}
NET_DSA_END