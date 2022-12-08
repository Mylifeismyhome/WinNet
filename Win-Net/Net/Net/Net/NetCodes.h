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

#pragma once
#define NET_ERROR_LIST_BEGIN void Net::Codes::NetLoadErrorCodes() { if(__bloaded_NetCodes) return;
#define NET_ERROR_LIST_END __bloaded_NetCodes = true; }
#define NET_ERROR_OBJ static std::vector<Net::Codes::NetErrorCode_T> NetErrorCode_L
#define NET_CHECK_LOADED_VAR static bool __bloaded_NetCodes = false
#define NET_DEFINE_ERROR(code, message) Net::Codes::NetDefineErrorMessage(code, CSTRING(message))

#define NET_ERROR_CODE Net::Codes::NET_ERROR_CODE_T
#define NET_ERROR_LAST_CODE Net::Codes::NET_ERROR_CODE_T::LAST_NET_ERROR_CODE

#define NET_DEFINE_ERROR_CODE(name, ...) \
namespace name { \
enum T##name \
{ \
__VA_ARGS__ \
}; \
}

#include <Net/Net/NetString.h>

namespace Net
{
	namespace Codes
	{
		NET_DSA_BEGIN

		struct NetErrorCode_T
		{
			int Code;
			char* Message;

			NetErrorCode_T(const int Code, const char* Message)
			{
				this->Code = Code;

				const auto slen = strlen(Message);
				this->Message = ALLOC<char>(slen + 1);
				memcpy(this->Message, Message, slen);
				this->Message[slen] = '\0';
			}

			void free()
			{
				FREE<char>(Message);
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
			NET_ERR_Version,
			NET_ERR_UndefinedFrame,
			NET_ERR_InvalidFrameHeader,
			NET_ERR_InvalidFrameFooter,
			NET_ERR_DecryptAES,
			NET_ERR_DataInvalid,
			NET_ERR_NoMemberID,
			NET_ERR_MemberIDInvalid,
			NET_ERR_NoMemberContent,

			LAST_NET_ERROR_CODE
		};

		void NetDefineErrorMessage(int, const char*);
		void NetLoadErrorCodes();
		void NetUnloadErrorCodes();
		const char* NetGetErrorMessage(int);

		NET_DSA_END
	}
}
