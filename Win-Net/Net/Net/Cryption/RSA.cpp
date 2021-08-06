#include "RSA.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
NETRSA::NETRSA()
{
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	ERR_load_crypto_strings();

	KeyPair = nullptr;
	_PublicKey = nullptr;
	_PrivateKey = nullptr;
	Set = false;
}

NETRSA::~NETRSA()
{
	RSA_free(KeyPair);
	_PublicKey.free();
	_PrivateKey.free();
}

void NETRSA::generateKeyPair(const size_t num_bits, const int e)
{
	RSA_free(KeyPair);
	KeyPair = RSA_generate_key(static_cast<int>(num_bits), e, nullptr, nullptr);
}

bool NETRSA::generateKeys(const size_t num_bits, const int e)
{
	generateKeyPair(num_bits, e);

	// Obtain Public Key
	{
		/* To get the C-string PEM form: */
		const auto bio = BIO_new(BIO_s_mem());
		if (!bio)
		{
			LOG_ERROR(CSTRING("Unable to allocate new BIO"));
			return false;
		}

		PEM_write_bio_RSA_PUBKEY(bio, KeyPair);
		const auto len = BIO_pending(bio);

		CPOINTER<char> key(ALLOC<char>(static_cast<size_t>(len) + 1));
		BIO_read(bio, key.get(), len);
		key.get()[len] = '\0';

		_PublicKey.init(key.get());
		key.free();

		BIO_free(bio);
	}

	// Obtain Private Key
	{
		/* To get the C-string PEM form: */
		const auto bio = BIO_new(BIO_s_mem());
		if (!bio)
		{
			LOG_ERROR(CSTRING("Unable to allocate new BIO"));
			return false;
		}

		PEM_write_bio_RSAPrivateKey(bio, KeyPair, nullptr, nullptr, 0, nullptr, nullptr);
		const auto len = BIO_pending(bio);

		CPOINTER<char> key(ALLOC<char>(static_cast<size_t>(len) + 1));
		BIO_read(bio, key.get(), len);
		key.get()[len] = '\0';

		_PrivateKey.init(key.get());
		key.free();

		BIO_free(bio);
	}

	return true;
}

XOR_UNIQUEPOINTER NETRSA::publicKey()
{
	return _PublicKey.revert();
}

XOR_UNIQUEPOINTER NETRSA::privateKey()
{
	return _PrivateKey.revert();
}

void NETRSA::setPublicKey(char* key)
{
	_PublicKey.free();
	_PublicKey.init(key);
}

void NETRSA::setPublicKey(const char* key)
{
        _PublicKey.free();
        _PublicKey.init(key);
}

void NETRSA::setPrivateKey(char* key)
{
	_PrivateKey.free();
	_PrivateKey.init(key);
}

void NETRSA::setPrivateKey(const char* key)
{
        _PrivateKey.free();
        _PrivateKey.init(key);
}

void NETRSA::deleteKeys()
{
	if (KeyPair)
	{
		RSA_free(KeyPair);
		KeyPair = nullptr;
	}

	_PublicKey.free();
	_PrivateKey.free();
}

bool NETRSA::init(const char* in_PublicKey, const char* in_PrivateKey)
{
	if (!in_PublicKey
		|| !in_PrivateKey)
		return false;

	_PublicKey.init(in_PublicKey);
	_PrivateKey.init(in_PrivateKey);
	Set = true;
	return true;
}

bool NETRSA::init(char* in_PublicKey, char* in_PrivateKey)
{
	if (!in_PublicKey
		|| !in_PrivateKey)
		return false;

	_PublicKey.init(in_PublicKey);
	_PrivateKey.init(in_PrivateKey);
	Set = true;
	return true;
}

bool NETRSA::encrypt(CryptoPP::byte*& data, size_t& size)
{
	const auto bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Unable to allocate new BIO"));
		return false;
	}

	const auto uniquePointer = publicKey();
	BIO_write(bio, uniquePointer.get(), static_cast<int>(uniquePointer.length()));

	EVP_PKEY* pkey = nullptr;
	PEM_read_bio_PUBKEY(bio, &pkey, nullptr, nullptr);
	if (pkey)
	{
		const auto publicKey = EVP_PKEY_get1_RSA(pkey);
		EVP_PKEY_free(pkey);

		if (publicKey != nullptr)
		{
			const auto tmpEncSize = RSA_size((const RSA*)publicKey);
			auto  tmpEnc = ALLOC<byte>(static_cast<size_t>(tmpEncSize) + 1);
			const auto  encLen = RSA_public_encrypt(static_cast<int>(size), data, tmpEnc, publicKey, RSA_PKCS1_OAEP_PADDING);
			if (encLen > 0)
			{
				FREE(data);
				data = tmpEnc; // pointer swap
				data[encLen] = '\0';
				size = encLen;

				BIO_free(bio);
				RSA_free(publicKey);
				return true;
			}

			LOG(CSTRING("RSA has been failed: %s"), ERR_reason_error_string(ERR_get_error()));
			FREE(tmpEnc);
			RSA_free(publicKey);
		}
	}

	BIO_free(bio);
	return false;
}

bool NETRSA::encryptHex(CryptoPP::byte*& data, size_t& size)
{
	if (encrypt(data, size))
	{
		NET_HEX::encode(data, size);
		return true;
	}

	return false;
}

bool NETRSA::encryptBase64(CryptoPP::byte*& data, size_t& size)
{
	if (encrypt(data, size))
	{
		NET_BASE64::encode(data, size);
		return true;
	}

	return false;
}

bool NETRSA::decrypt(CryptoPP::byte*& data, size_t& size)
{
	const auto bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Unable to allocate new BIO"));
		return false;
	}

	const auto uniquePointer = privateKey();
	BIO_write(bio, uniquePointer.get(), static_cast<int>(uniquePointer.length()));

	EVP_PKEY* pkey = nullptr;
	PEM_read_bio_PrivateKey(bio, &pkey, nullptr, nullptr);
	if (pkey != nullptr)
	{
		const auto privateKey = EVP_PKEY_get1_RSA(pkey);
		EVP_PKEY_free(pkey);

		if (privateKey != nullptr)
		{
			const auto tmpDecSize = RSA_size((const RSA*)privateKey);
			auto  tmpDec = ALLOC<byte>(static_cast<size_t>(tmpDecSize) + 1);
			const auto  decLen = RSA_private_decrypt(static_cast<int>(size), data, tmpDec, privateKey, RSA_PKCS1_OAEP_PADDING);
			if (decLen > 0)
			{
				FREE(data);
				data = tmpDec; // pointer swap
				data[decLen] = '\0';
				size = decLen;

				BIO_free(bio);
				RSA_free(privateKey);
				return true;
			}

			LOG(CSTRING("RSA has been failed: %s"), ERR_reason_error_string(ERR_get_error()));
			FREE(tmpDec);
			RSA_free(privateKey);
		}
	}

	BIO_free(bio);
	return false;
}

bool NETRSA::decryptHex(CryptoPP::byte*& data, size_t& size)
{
	NET_HEX::decode(data, size);

	if (decrypt(data, size))
		return true;

	return false;
}

bool NETRSA::decryptBase64(CryptoPP::byte*& data, size_t& size)
{
	NET_BASE64::decode(data, size);

	if (decrypt(data, size))
		return true;

	return false;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
