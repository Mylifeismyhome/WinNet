#pragma once
#include <Net/Net/Net.h>
#include <NetClient/Client.h>
#include "..//..//..//Packet.hpp"

NET_CLASS(Client, NET_CLIENT)
{
	NET_CALLBACK(void, OnConnected);
	NET_CALLBACK(void, OnDisconnected);
	NET_CALLBACK(void, OnForcedDisconnect, int);
	NET_CALLBACK(void, OnTimeout);
	NET_CALLBACK(void, OnKeysReceived);
	NET_CALLBACK(void, OnKeysFailed);
	NET_CALLBACK(void, OnConnectionEstabilished);
	NET_CALLBACK(void, OnVersionMismatch);
	NET_CALLBACK(bool, CheckData, int, NET_PACKET&);

	NET_HANDLE_TIMER hTimer;

	NET_DECLARE_PACKET(Test);
};
