#include "Client.h"
#include <Net/assets/thread.h>

NET_DECLARE_PACKET_CALLBACK_BEGIN(Client)
NET_DEFINE_PACKET_CALLBACK(Test, Packages::PKG_TEST)
NET_DECLARE_PACKET_CALLBACK_END

NET_TIMER(Test)
{
	const auto client = (Client*)param;
	if (!client) NET_STOP_TIMER;

	NET_PACKET pkg;
	pkg[CSTRING("text")] = CSTRING("WinNet");
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

NET_BEGIN_PACKET(Client, Test)
LOG("Received Package from Server");
NET_END_PACKET