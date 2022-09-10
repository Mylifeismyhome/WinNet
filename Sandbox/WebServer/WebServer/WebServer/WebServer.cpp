#include "WebServer.h"

NET_DECLARE_PACKET_CALLBACK_BEGIN(WebServer)
NET_DECLARE_PACKET_CALLBACK_END

void WebServer::Tick() {}
void WebServer::OnPeerConnect(NET_PEER peer) 
{
	LOG(CSTRING("CONNECTED"));
}
void WebServer::OnPeerEstabilished(NET_PEER peer) {}
void WebServer::OnPeerDisconnect(NET_PEER peer, int reason) {}
void WebServer::OnPeerUpdate(NET_PEER peer) {}