#pragma once
#include <Net/Net/Net.h>
#include <NetServer/Server.h>
#include "..//..//..//Packages.hpp"

NET_INHERITANCE(Server, NET_SERVER)
{
	NET_CALLBACK(void, Tick);
	NET_CALLBACK(void, OnPeerConnect, NET_PEER);
	NET_CALLBACK(void, OnPeerDisconnect, NET_PEER, int);
	NET_CALLBACK(void, OnPeerEstabilished, NET_PEER);
	NET_CALLBACK(void, OnPeerUpdate, NET_PEER);
	NET_CALLBACK(bool, CheckData, NET_PEER, int, NET_PACKET&);

	NET_CLASS_PUBLIC;
	NET_DEFINE_PACKET(Test);
};
