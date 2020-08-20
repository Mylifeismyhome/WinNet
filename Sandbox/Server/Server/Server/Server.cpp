#include "Server.h"

NET_SERVER_BEGIN_DATA_PACKAGE(Server)
NET_SERVER_DEFINE_PACKAGE(JulianStinkt, Packages::PKG_JulianStinkt)
NET_SERVER_END_DATA_PACKAGE

void Server::Tick()
{
}

void Server::OnPeerConnect(NET_PEER peer)
{
}

void Server::OnPeerEstabilished(NET_PEER peer)
{
	NET_FILEMANAGER fmanager(CSTRING("test.mp4"), NET_FILE_READ | NET_FILE_BINARY);

	byte* data = nullptr;
	size_t size = NULL;
	if(fmanager.read(data, size))
	{
		Package pkg;	
		pkg.AppendRawData(CSTRING("FILE"), data, size);
		//DoSend(peer, Packages::PKG_TEST, pkg);
	}
}

void Server::OnPeerDisconnect(NET_PEER peer)
{
}

void Server::OnPeerUpdate(NET_PEER peer)
{
}

void Server::OnJulianStinkt(NET_PEER peer, NET_PACKAGE pkg)
{
	const auto arg = pkg.String(CSTRING("Thor"));
	if (!arg.valid())
		return;

	LOG(CSTRING("Yes Julians Cock ist extreme: %s"), arg.value());
}