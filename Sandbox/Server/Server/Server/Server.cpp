#include "Server.h"

NET_SERVER_BEGIN_DATA_PACKAGE(Server)
NET_SERVER_END_DATA_PACKAGE

void Server::Tick()
{
	/*if(KEYWASPRESSED(KEYBOARD::F6))
	{
		NET_TEST_MEMORY_SHOW_DIAGNOSTIC();
	}*/
}

void Server::OnPeerConnect(NET_PEER peer)
{
}

void Server::OnPeerEstabilished(NET_PEER peer)
{
	//Package pkg;
	//NET_SEND(peer, Packages::PKG_TEST, pkg);
}

void Server::OnPeerDisconnect(NET_PEER peer)
{
}

void Server::OnPeerUpdate(NET_PEER peer)
{
}