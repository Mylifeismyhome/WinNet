#pragma once
#include <Net/Net/Net.h>
#include <NetWebSocket/WebSocket.h>

NET_CLASS(WebSocket, NET_WEB_SOCKET)
{
	NET_CALLBACK(void, Tick);
	NET_CALLBACK(void, OnPeerConnect, NET_PEER);
	NET_CALLBACK(void, OnPeerDisconnect, NET_PEER, int);
	NET_CALLBACK(void, OnPeerEstabilished, NET_PEER);
	NET_CALLBACK(void, OnPeerUpdate, NET_PEER);
	NET_CALLBACK(bool, CheckData, NET_PEER, int, NET_PACKET&);
};
