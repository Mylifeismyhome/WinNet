#include "Server.h"

NET_SERVER_BEGIN_DATA_PACKAGE(Server)
NET_SERVER_DEFINE_PACKAGE(Test, Packages::PKG_TEST);
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
	profile.Add(peer);
	auto info = profile.peer(peer);
	if (info)
		info->Append("Test", "Hallo du tester");
}

void Server::OnPeerEstabilished(NET_PEER peer)
{
	//Package pkg;
	//NET_SEND(peer, Packages::PKG_TEST, pkg);
}

void Server::OnPeerDisconnect(NET_PEER peer)
{
	auto info = profile.peer(peer);
	if (info)
	{
		auto str = info->String("Test");
		if (str.valid())
			LOG(str.value());
	}

	profile.Remove(peer);
}

void Server::OnPeerUpdate(NET_PEER peer)
{
}

NET_BEGIN_FNC_PKG(Server, Test)
//LOG("TEST PKG!!!");
NET_END_FNC_PKG