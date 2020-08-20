#pragma once
#define NET_ERROR_LIST_BEGIN static void NetLoadErrorCodes() {
#define NET_ERROR_LIST_END }
#define NET_ERROR_OBJ static std::vector<NetErrorCode_T> NetErrorCode_L
#define NET_DEFINE_ERROR(code, message) 	NetDefineErrorMessage(code, CSTRING(message))

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

static void NetDefineErrorMessage(const int code, const char* message)
{
	for (auto& val : NetErrorCode_L)
	{
		if (val.Code == code)
		{
			LOG_ERROR(CSTRING("Duplicated error coded: (%i, '%s')"), code, message);
			return;
		}
	}
	
	NetErrorCode_L.push_back(NetErrorCode_T(code, message));
}

NET_ERROR_LIST_BEGIN
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_CryptKeyBase64, "Failed to crypt AES Key or encode to Base64");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_CryptIVBase64, "Failed to crypt AES IV Key or encode to Base64");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_InitAES, "Failed to initialise AES");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_InitRSA, "Failed to initialise RSA");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_PackageKeySizeEmpty, "Received AES Key size is equal to NULL");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_PackageIVSizeEmpty, "Received IV Key size is equal to NULL");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_PackageSizeEmpty, "Received Package size is equal to NULL");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_PackageDataSizeEmpty, "Received Package data size is equal to NULL");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_DecryptKeyBase64, "Failed to decrypt AES Key or encode to Base64");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_DecryptIVBase64, "Failed to decrypt AES IV Key or encode to Base64");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_EmptyRSAPublicKey, "Received RSA Publickey size is equal to NULL");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_Versionmismatch, "Application version mismatch");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_Handshake, "Failed to perform TLS Handshake");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_UndefinedPackage, "Received Package is undefined");
NET_ERROR_LIST_END

static const char* NetGetErrorMessage(const int code)
{
	for (auto& val : NetErrorCode_L)
	{
		if (val.Code == code)
			return val.Message;
	}

	return CSTRING("UNDEFINED");
}

#define NET_DEFINE_ERROR_CODE(name, ...) \
NET_NAMESPACE_BEGIN(name) \
enum T##name \
{ \
__VA_ARGS__ \
}; \
NET_NAMESPACE_END

NET_DSA_END