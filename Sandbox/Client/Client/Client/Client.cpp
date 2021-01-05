#include "Client.h"
#include <Net/assets/thread.h>

NET_CLIENT_BEGIN_DATA_PACKAGE(Client)
NET_CLIENT_DEFINE_PACKAGE(Test, Packages::PKG_TEST)
NET_CLIENT_END_DATA_PACKAGE

NET_THREAD(Test)
{
	const auto client = (Client*)parameter;
	if (!client) return NULL;

	while (true)
	{
		Net::Package pkg;
		client->DoSend(Packages::PKG_TEST, pkg);

		Sleep(1000);
	}
}

void Client::OnConnected()
{
}

void Client::OnConnectionEstabilished()
{
	Net::Thread::Create(Test, this);
}

void Client::OnDisconnected()
{
}

void Client::OnForcedDisconnect(int)
{
}

void Client::OnKeysFailed()
{
}

void Client::OnKeysReceived()
{
}

void Client::OnTimeout()
{
}

void Client::OnVersionMismatch()
{
}

NET_BEGIN_FNC_PKG(Client, Test)
LOG("A");
NET_END_FNC_PKG