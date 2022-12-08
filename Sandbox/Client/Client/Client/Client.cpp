#include "Client.h"
#include <Net/assets/thread.h>

NET_PACKET_DEFINITION_BEGIN(Client)
NET_DEFINE_PACKET(Test, Sandbox::Packet::PKG_TEST)
NET_PACKET_DEFINITION_END

NET_TIMER(Test)
{
	const auto client = (Client*)param;
	if (!client) NET_STOP_TIMER;

	NET_PACKET pkg;
	pkg[CSTRING("text")] = CSTRING("WinNet");
	client->NET_SEND(Sandbox::Packet::PKG_TEST, pkg);

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

void Client::OnConnectionClosed(int) {}
void Client::OnKeysFailed() {}
void Client::OnKeysReceived() {}
void Client::OnVersionMismatch() {}

NET_BEGIN_PACKET(Client, Test)
NET_LOG("Received Packet from Server");
NET_END_PACKET