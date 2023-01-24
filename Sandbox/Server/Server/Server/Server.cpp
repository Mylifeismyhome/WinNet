#include "Server.h"

#include <Net/assets/manager/filemanager.h>

NET_PACKET_DEFINITION_BEGIN(Server)
NET_DEFINE_PACKET(Test, Sandbox::Packet::PKG_TEST);
NET_DEFINE_PACKET(TestRaw, Sandbox::Packet::PKG_TEST_RAW);
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

NET_BEGIN_PACKET(Server, TestRaw)
auto image = PKG.GetRaw("Image");
if (!image)
{
	return;
}

NET_FILEMANAGERA fm("test_out.jpg", NET_FILE_WRITE);
fm.write(image->value(), image->size());
NET_END_PACKET
