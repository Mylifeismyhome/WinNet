#pragma once
#include <Net/Net/Net.h>
#include <NetClient/Client.h>
#include "..//..//..//Packages.hpp"

NET_INHERITANCE(Client, NET_CLIENT)
{
	NET_CALLBACK(void, OnConnected);
	NET_CALLBACK(void, OnDisconnected);
	NET_CALLBACK(void, OnForcedDisconnect, int);
	NET_CALLBACK(void, OnTimeout);
	NET_CALLBACK(void, OnKeysReceived);
	NET_CALLBACK(void, OnKeysFailed);
	NET_CALLBACK(void, OnConnectionEstabilished);
	NET_CALLBACK(void, OnVersionMismatch);
	NET_CALLBACK(bool, CheckData, int, NET_PACKAGE);

	NET_HANDLE_TIMER hTimer;

	NET_DEF_FNC_PKG(Test);
};