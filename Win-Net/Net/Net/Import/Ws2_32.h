#pragma once
#include <Net/Net/Net.h>
#include <Net/Import/Definition.hpp>
#include <winternl.h>

namespace Net
{
	IMPORT_BEGIN(Ws2_32);
	IMPORT_DEFINE_FNC_WINAPI(int, send, SOCKET, const char*, int, int);
	IMPORT_DEFINE_FNC_WINAPI(int, recv, SOCKET, const char*, int, int);
	IMPORT_END
};