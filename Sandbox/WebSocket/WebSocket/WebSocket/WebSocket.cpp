#include "WebSocket.h"

NET_DECLARE_PACKET_CALLBACK_BEGIN(WebSocket)
NET_DECLARE_PACKET_CALLBACK_END

void WebSocket::Tick() {}
void WebSocket::OnPeerConnect(NET_PEER peer)
{
	NET_LOG(CSTRING("CONNECTED"));
}
void WebSocket::OnPeerEstabilished(NET_PEER peer) {}
void WebSocket::OnPeerDisconnect(NET_PEER peer, int reason) {}
void WebSocket::OnPeerUpdate(NET_PEER peer) {}