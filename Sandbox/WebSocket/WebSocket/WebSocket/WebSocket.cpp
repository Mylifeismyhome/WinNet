#include "WebSocket.h"

NET_PACKET_DEFINITION_BEGIN(WebSocket)
NET_DEFINE_PACKET(NetReplyMessage, 0);
NET_PACKET_DEFINITION_END

void WebSocket::Tick() {}
void WebSocket::OnPeerConnect(NET_PEER peer)
{
	NET_LOG(CSTRING("CONNECTED"));
}
void WebSocket::OnPeerEstabilished(NET_PEER peer) {}
void WebSocket::OnPeerDisconnect(NET_PEER peer, int reason) {}
void WebSocket::OnPeerUpdate(NET_PEER peer) {}

NET_BEGIN_PACKET(WebSocket, NetReplyMessage)
if (!PKG[CSTRING("text")])
{
	return;
}
auto text = PKG[CSTRING("text")]->as_string();
NET_LOG(CSTRING("Client send message '%s'"), text);

// reply message
NET_SEND(peer, 0, PKG);
NET_END_PACKET