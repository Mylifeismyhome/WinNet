#pragma once
#define NET_ERROR_LIST_BEGIN void Net::Codes::NetLoadErrorCodes() {
#define NET_ERROR_LIST_END }
#define NET_ERROR_OBJ static std::vector<NetErrorCode_T> NetErrorCode_L
#define NET_DEFINE_ERROR(code, message) Net::Codes::NetDefineErrorMessage(code, CSTRING(message))

#define NET_ERROR_CODE NET_ERROR_CODE_T
#define NET_ERROR_LAST_CODE NET_ERROR_CODE_T::LAST_NET_ERROR_CODE

#include <Net/NetString.h>

NET_DSA_BEGIN

/* Macros to all Net Codes */
struct NetErrorCode_T
{
	int Code;
	char Message[MAX_PATH];

	NetErrorCode_T(const int Code, const char* Message)
	{
		this->Code = Code;
		strcpy_s(this->Message, Message);
	}
};

enum NET_ERROR_CODE_T
{
	NET_ERR_CryptKeyBase64 = 0x1,
	NET_ERR_CryptIVBase64,
	NET_ERR_InitAES,
	NET_ERR_InitRSA,
	NET_ERR_PackageKeySizeEmpty,
	NET_ERR_PackageIVSizeEmpty,
	NET_ERR_PackageSizeEmpty,
	NET_ERR_PackageDataSizeEmpty,
	NET_ERR_DecryptKeyBase64,
	NET_ERR_DecryptIVBase64,
	NET_ERR_EmptyRSAPublicKey,
	NET_ERR_Versionmismatch,
	NET_ERR_Handshake,
	NET_ERR_UndefinedPackage,

	LAST_NET_ERROR_CODE
};

NET_ERROR_OBJ;

namespace Net
{
	namespace Codes
	{
		void NetDefineErrorMessage(int, const char*);
		void NetLoadErrorCodes();
		const char* NetGetErrorMessage(int);
	}
}

#define NET_DEFINE_ERROR_CODE(name, ...) \
NET_NAMESPACE_BEGIN(name) \
enum T##name \
{ \
__VA_ARGS__ \
}; \
NET_NAMESPACE_END

NET_DSA_END