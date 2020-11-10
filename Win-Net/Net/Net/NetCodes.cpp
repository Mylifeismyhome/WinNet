#include <Net/NetCodes.h>
#include <assets/manager/logmanager.h>

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

void Net::Codes::NetDefineErrorMessage(const int code, const char* message)
{
	for (auto& val : NetErrorCode_L)
	{
		if (val.Code == code)
			return;
	}

	NetErrorCode_L.emplace_back(NetErrorCode_T(code, message));
}

const char* Net::Codes::NetGetErrorMessage(const int code)
{
	for (auto& val : NetErrorCode_L)
	{
		if (val.Code == code)
			return val.Message;
	}

	return CSTRING("UNDEFINED");
}