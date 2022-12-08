/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#include <Net/Net/NetCodes.h>
#include <Net/assets/manager/logmanager.h>

NET_ERROR_OBJ;
NET_CHECK_LOADED_VAR;

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
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_UndefinedFrame, "Frame is undefined and can not be processed");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_InvalidFrameHeader, "Frame header is not valid");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_InvalidFrameFooter, "Frame footer is not valid");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_DecryptAES, "Failed to decrypt AES Frame");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_DataInvalid, "Data of frame is invalid");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_NoMemberID, "Missing member ID in frame");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_MemberIDInvalid, "Member ID is less than zero");
NET_DEFINE_ERROR(NET_ERROR_CODE::NET_ERR_NoMemberContent, "Missing member Content in frame");
NET_ERROR_LIST_END

void Net::Codes::NetUnloadErrorCodes()
{
	for (auto& entry : NetErrorCode_L)
		entry.free();

	NetErrorCode_L.clear();

	__bloaded_NetCodes = false;
}

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