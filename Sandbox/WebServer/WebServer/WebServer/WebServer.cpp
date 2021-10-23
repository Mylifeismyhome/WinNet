#include "WebServer.h"

NET_SERVER_BEGIN_DATA_PACKAGE(WebServer)
NET_SERVER_END_DATA_PACKAGE

void WebServer::Tick() {}
void WebServer::OnPeerConnect(NET_PEER peer) 
{
	LOG(CSTRING("CONNECTED"));
}
void WebServer::OnPeerEstabilished(NET_PEER peer) {}
void WebServer::OnPeerDisconnect(NET_PEER peer, int reason) {}
void WebServer::OnPeerUpdate(NET_PEER peer) {}