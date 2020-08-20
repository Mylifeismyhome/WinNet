#pragma once
/* Macros to all Net Codes */
struct NetErrorCode_T
{
	int Code;
	char Message[256];
};

enum NetErrorCodes
{
	NET_ERR_CryptKeyBase64 = 0x000001,
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

static NetErrorCode_T NetErrorCode_L[] = 
{
	{NetErrorCodes::NET_ERR_CryptKeyBase64, "Failed to crypt AES Key or encode to Base64"},
	{NetErrorCodes::NET_ERR_CryptIVBase64, "Failed to crypt AES IV Key or encode to Base64"},
	{NetErrorCodes::NET_ERR_InitAES, "Failed to initialise AES"},
	{NetErrorCodes::NET_ERR_InitRSA, "Failed to initialise RSA"},
	{NetErrorCodes::NET_ERR_PackageKeySizeEmpty, "Received AES Key size is equal to NULL"},
	{NetErrorCodes::NET_ERR_PackageIVSizeEmpty, "Received IV Key size is equal to NULL"},
	{NetErrorCodes::NET_ERR_PackageSizeEmpty, "Received Package size is equal to NULL"},
	{NetErrorCodes::NET_ERR_PackageDataSizeEmpty, "Received Package data size is equal to NULL"},
	{NetErrorCodes::NET_ERR_DecryptKeyBase64, "Failed to decrypt AES Key or encode to Base64"},
	{NetErrorCodes::NET_ERR_DecryptIVBase64, "Failed to decrypt AES IV Key or encode to Base64"},
	{NetErrorCodes::NET_ERR_EmptyRSAPublicKey, "Received RSA Publickey size is equal to NULL"},
	{NetErrorCodes::NET_ERR_Versionmismatch, "Application version mismatch"},
	{NetErrorCodes::NET_ERR_Handshake, "Failed to perform TLS Handshake"},
	{NetErrorCodes::NET_ERR_UndefinedPackage, "Received Package is undefined"},
};

static char* NetGetErrorMessage(const int code)
{
	for (auto& val : NetErrorCode_L)
	{
		if (val.Code == code)
			return val.Message;
	}

	return (char*)"UNKNOWN";
}