#include "Client.h"

NET_CLIENT_BEGIN_DATA_PACKAGE(Client)
NET_CLIENT_DEFINE_PACKAGE(Test, Packages::PKG_TEST)
NET_CLIENT_END_DATA_PACKAGE

void Client::Tick()
{
}

void Client::OnConnected()
{
}

void Client::OnConnectionEstabilished()
{
}

void Client::OnDisconnected()
{
}

void Client::OnForcedDisconnect(int)
{
}

void Client::OnKeysFailed()
{
}

void Client::OnKeysReceived()
{
}

void Client::OnTimeout()
{
}

void Client::OnVersionMismatch()
{
}

NET_BEGIN_FNC_PKG(Client, Test)
LOG("A");
NET_END_FNC_PKG