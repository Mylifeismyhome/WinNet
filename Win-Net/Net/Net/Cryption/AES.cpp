#include "AES.h"

namespace Net
{
	namespace Cryption
	{
		AES::AES()
		{
			IV = nullptr;
			Key = nullptr;
			KeyLength = NULL;
		}

		AES::~AES()
		{
			IV.free();
			Key.free();
		}

		bool AES::init(const char* in_Key, const char* in_IV, const size_t in_KeyLength)
		{
			if (!in_Key || !in_IV)
				return false;

			Key = RUNTIMEXOR((char*)in_Key);
			IV = RUNTIMEXOR((char*)in_IV);
			KeyLength = in_KeyLength;
			return true;
		}

		bool AES::init(char* in_Key, char* in_IV, const size_t in_KeyLength)
		{
			if (!in_Key || !in_IV)
				return false;

			Key = RUNTIMEXOR(in_Key);
			IV = RUNTIMEXOR(in_IV);
			KeyLength = in_KeyLength;
			return true;
		}

		bool AES::encrypt(CryptoPP::byte* data, const size_t size, const char* key, const char* iv) const
		{
			try
			{
				CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryptor(reinterpret_cast<const CryptoPP::byte*>(key), KeyLength, reinterpret_cast<const CryptoPP::byte*>(iv));
				encryptor.ProcessData(data, data, size);
				return true;
			}
			catch (const CryptoPP::Exception& ex)
			{
				NET_LOG_ERROR(CSTRING("[NET_AES][ENCRPYT] - %s"), ex.what());
				return false;
			}
		}

		bool AES::encrypt(CryptoPP::byte* data, const size_t size)
		{
			return encrypt(data, size, Key.revert().get(), IV.revert().get());
		}

		bool AES::encryptHex(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			if (encrypt(data, size))
				return NET_HEX::encode(data, out, size);

			return true;
		}

		bool AES::encryptHex(CryptoPP::byte*& data, size_t& size)
		{
			if (encrypt(data, size))
				return NET_HEX::encode(data, size);

			return true;
		}

		bool AES::encryptBase64(CryptoPP::byte*& data, size_t& size)
		{
			if (encrypt(data, size))
			{
				NET_BASE64::encode(data, size);
				return true;
			}

			return false;
		}

		bool AES::encryptBase64(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			if (encrypt(data, size))
			{
				NET_BASE64::encode(data, out, size);
				FREE(data);
				return true;
			}

			return false;
		}

		bool AES::decrypt(CryptoPP::byte* data, const size_t size, const char* key, const char* iv) const
		{
			try
			{
				CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(reinterpret_cast<const CryptoPP::byte*>(key), KeyLength, reinterpret_cast<const CryptoPP::byte*>(iv));
				decryptor.ProcessData(data, data, size);
				return true;
			}
			catch (const CryptoPP::Exception& ex)
			{
				NET_LOG_ERROR(CSTRING("[NET_AES][DECRPYT] - %s"), ex.what());
				return false;
			}
		}

		bool AES::decrypt(CryptoPP::byte* data, const size_t size)
		{
			return decrypt(data, size, Key.revert().get(), IV.revert().get());
		}

		bool AES::decryptHex(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			if (NET_HEX::decode(data, out, size))
			{
				if (decrypt(out, size))
					return true;
			}

			return false;
		}

		bool AES::decryptHex(CryptoPP::byte*& data, size_t& size)
		{
			if (NET_HEX::decode(data, size))
			{
				if (decrypt(data, size))
					return true;
			}

			return false;
		}

		bool AES::decryptBase64(CryptoPP::byte*& data, size_t& size)
		{
			NET_BASE64::decode(data, size);
			if (decrypt(data, size))
				return true;

			return false;
		}

		bool AES::decryptBase64(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
		{
			NET_BASE64::decode(data, out, size);
			if (decrypt(out, size))
			{
				FREE(data);
				return true;
			}

			return false;
		}
	}
}