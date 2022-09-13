#include "WebSocket.h"

NET_PACKET_DEFINITION_BEGIN(WebSocket)
NET_PACKET_DEFINITION_END

void WebSocket::Tick() {}
void WebSocket::OnPeerConnect(NET_PEER peer)
{
	NET_LOG(CSTRING("CONNECTED"));
}
void WebSocket::OnPeerEstabilished(NET_PEER peer) {}
void WebSocket::OnPeerDisconnect(NET_PEER peer, int reason) {}
void WebSocket::OnPeerUpdate(NET_PEER peer) {}