#include "Client.h"
#include <Net/assets/thread.h>

NET_CLIENT_BEGIN_DATA_PACKAGE(Client)
NET_CLIENT_DEFINE_PACKAGE(Test, Packages::PKG_TEST)
NET_CLIENT_END_DATA_PACKAGE

NET_TIMER(Test)
{
	const auto client = (Client*)param;
	if (!client) NET_STOP_TIMER;

	Net::Package::Package pkg;
	pkg.Append(CSTRING("years"), 53);
	client->DoSend(Packages::PKG_TEST, pkg);

	NET_CONTINUE_TIMER;
}

void Client::OnConnected() {}

void Client::OnConnectionEstabilished()
{
	hTimer = Net::Timer::Create(Test, 1000, this);
}

void Client::OnDisconnected()
{
	Net::Timer::Clear(hTimer);
}

void Client::OnForcedDisconnect(int) {}
void Client::OnKeysFailed() {}
void Client::OnKeysReceived() {}
void Client::OnTimeout() {}
void Client::OnVersionMismatch() {}

NET_BEGIN_FNC_PKG(Client, Test)
LOG("Received Package from Server");
NET_END_FNC_PKG
