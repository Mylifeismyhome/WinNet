#pragma once
#include <Net/Net/Net.h>

#ifdef NET_DISABLE_IMPORT_USER32
#define User32
#else
#include <Net/Import/Definition.hpp>
#include <winternl.h>

namespace Net
{
	IMPORT_BEGIN_HEAD(User32);
	IMPORT_DEFINE_FNC_WINAPI(int, MessageBoxA, HWND, LPCSTR, LPCSTR, UINT);
	IMPORT_DEFINE_FNC_WINAPI(int, MessageBoxW, HWND, LPCWSTR, LPCWSTR, UINT);
	IMPORT_END
};
#endif
