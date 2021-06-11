#include "Server.h"

NET_SERVER_BEGIN_DATA_PACKAGE(Server)
NET_SERVER_DEFINE_PACKAGE(Test, Packages::PKG_TEST);
NET_SERVER_END_DATA_PACKAGE

void Server::Tick() {}
void Server::OnPeerConnect(NET_PEER peer) {}
void Server::OnPeerEstabilished(NET_PEER peer) {}
void Server::OnPeerDisconnect(NET_PEER peer) {}
void Server::OnPeerUpdate(NET_PEER peer) {}

NET_BEGIN_FNC_PKG(Server, Test)
LOG("Received Package from Client");

DoSend(PEER, Packages::PKG_TEST, pkg);
NET_END_FNC_PKG