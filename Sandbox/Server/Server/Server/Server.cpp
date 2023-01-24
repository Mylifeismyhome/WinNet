#include "Server.h"

NET_PACKET_DEFINITION_BEGIN(Server)
NET_DEFINE_PACKET(Test, Sandbox::Packet::PKG_TEST);
NET_PACKET_DEFINITION_END

void Server::Tick() {}
void Server::OnPeerConnect(NET_PEER peer) {}
void Server::OnPeerEstabilished(NET_PEER peer) {}
void Server::OnPeerDisconnect(NET_PEER peer, int reason) {}
void Server::OnPeerUpdate(NET_PEER peer) {}

NET_BEGIN_PACKET(Server, Test)
if (!PKG[CSTRING("text")])
{
	return;
}
auto text = PKG[CSTRING("text")]->as_string();
NET_LOG(CSTRING("Hello '%s' from Client"), text);

NET_PACKET reply;
NET_SEND(peer, Sandbox::Packet::PKG_TEST, reply);
NET_END_PACKET
