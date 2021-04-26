#pragma once
#include <Net/Net/Net.h>
#include <NetServer/Server.h>
#include "..//..//..//Packages.hpp"
#include "Net/assets/manager/profile.hpp"

NET_INHERITANCE(Server, NET_SERVER)
{
	NET_CALLBACK(void, Tick);
	NET_CALLBACK(void, OnPeerConnect, NET_PEER);
	NET_CALLBACK(void, OnPeerDisconnect, NET_PEER);
	NET_CALLBACK(void, OnPeerEstabilished, NET_PEER);
	NET_CALLBACK(void, OnPeerUpdate, NET_PEER);
	NET_CALLBACK(bool, CheckData, NET_PEER, int, NET_PACKAGE);

	Net::Manager::Profile<NET_PEER> profile;

	NET_CLASS_PUBLIC;
	NET_DEF_FNC_PKG(Test);
};