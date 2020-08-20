#include "RSA.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
NETRSA::NETRSA()
{
	OpenSSL_add_all_algorithms();
	OpenSSL_add_all_ciphers();
	ERR_load_crypto_strings();

	keypair = nullptr;
	PublicKey = nullptr;
	PrivateKey = nullptr;
	Set = false;
}

NETRSA::~NETRSA()
{
	PublicKey.free();
	PrivateKey.free();
	keypair.free();
}

bool NETRSA::GenerateKeys(const size_t num_bits, const int e)
{
	if (!keypair.valid())
		keypair = RSA_generate_key(static_cast<int>(num_bits), e, nullptr, nullptr);

	/* To get the C-string PEM form: */
	auto bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Failure on allocating BIO"));
		return false;
	}

	PEM_write_bio_RSA_PUBKEY(bio, keypair.get());

	auto pk = ALLOC<char>(num_bits + 1);
	BIO_read(bio, pk, static_cast<int>(num_bits));

	auto end = std::string(pk).find_last_of(NET_RSA_END) + 1;
	pk[end] = '\0';

	BIO_free(bio);

	PublicKey = RUNTIMEXOR(pk);
	FREE(pk);

	////////////////////

	/* To get the C-string PEM form: */
	bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Failure on allocating BIO"));
		return false;
	}

	PEM_write_bio_RSAPrivateKey(bio, keypair.get(), nullptr, nullptr, 0, nullptr, nullptr);

	auto pik = ALLOC<char>(num_bits + 1);
	BIO_read(bio, pik, static_cast<int>(num_bits));

	end = std::string(pik).find_last_of(NET_RSA_END) + 1;
	pik[end] = '\0';

	BIO_free(bio);

	PrivateKey = RUNTIMEXOR(pik);
	FREE(pik);

	return true;
}

char* NETRSA::CreatePublicKey(const size_t num_bits, const int e)
{
	if (!keypair.valid())
		keypair = RSA_generate_key(static_cast<int>(num_bits), e, nullptr, nullptr);

	/* To get the C-string PEM form: */
	const auto bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Failure on allocating BIO"));
		return nullptr;
	}

	PEM_write_bio_RSA_PUBKEY(bio, keypair.get());

	const auto pk = ALLOC<char>(num_bits + 1);
	BIO_read(bio, pk, static_cast<int>(num_bits));

	const auto end = std::string(pk).find_last_of(NET_RSA_END) + 1;
	pk[end] = '\0';

	BIO_free(bio);

	return pk;
}

char* NETRSA::CreatePrivateKey(const size_t num_bits, const int e)
{
	if (!keypair.valid())
		keypair = RSA_generate_key(static_cast<int>(num_bits), e, nullptr, nullptr);

	/* To get the C-string PEM form: */
	const auto bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Failure on allocating BIO"));
		return nullptr;
	}

	PEM_write_bio_RSAPrivateKey(bio, keypair.get(), nullptr, nullptr, 0, nullptr, nullptr);

	const auto pik = ALLOC<char>(num_bits + 1);
	BIO_read(bio, pik, static_cast<int>(num_bits));

	const auto end = std::string(pik).find_last_of(NET_RSA_END) + 1;
	pik[end] = '\0';

	BIO_free(bio);

	return pik;
}

bool NETRSA::Init(char* in_PublicKey, char* in_PrivateKey)
{
	if (!in_PublicKey
		|| !in_PrivateKey)
		return false;

	PublicKey = RUNTIMEXOR(in_PublicKey);
	PrivateKey = RUNTIMEXOR(in_PrivateKey);
	Set = true;
	return true;
}

bool NETRSA::encryptString(CryptoPP::byte** data, size_t& size)
{
	const auto bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Failure on allocating BIO"));
		return false;
	}

	BIO_write(bio, PublicKey.Revert().get(), static_cast<int>(PublicKey.length()));

	EVP_PKEY* pkey = nullptr;
	PEM_read_bio_PUBKEY(bio, &pkey, nullptr, nullptr);
	if (pkey != nullptr)
	{
		const auto publicKey = EVP_PKEY_get1_RSA(pkey);

		if (publicKey != nullptr)
		{
			const auto tmpEncSize = RSA_size((const RSA*)publicKey);
			auto  tmpEnc = ALLOC<byte>(static_cast<size_t>(tmpEncSize) + 1);
			const auto  encLen = RSA_public_encrypt(static_cast<int>(size), *data, tmpEnc, publicKey, RSA_PKCS1_OAEP_PADDING);
			if (encLen > 0)
			{
				FREE(*data);
				*data = tmpEnc; // pointer swap
				data[0][encLen] = '\0';
				size = encLen;

				BIO_free(bio);
				RSA_free(publicKey);
				free(pkey);
				return true;
			}

			LOG(CSTRING("RSA failure: %s"), ERR_reason_error_string(ERR_get_error()));
			FREE(tmpEnc);
			RSA_free(publicKey);
		}

		free(pkey);
	}

	BIO_free(bio);
	return false;
}

bool NETRSA::encryptStringHex(CryptoPP::byte** data, size_t& size)
{
	if (encryptString(data, size))
	{
		const NET_HEX hex;
		hex.encodeString(data, size);
		return true;
	}

	return false;
}

bool NETRSA::encryptStringBase64(CryptoPP::byte** data, size_t& size)
{
	if (encryptString(data, size))
	{
		NET_BASE64 base64;
		base64.encodeString(data, size);
		return true;
	}

	return false;
}

bool NETRSA::decryptString(CryptoPP::byte** data, size_t& size)
{
	const auto bio = BIO_new(BIO_s_mem());
	if (!bio)
	{
		LOG_ERROR(CSTRING("Failure on allocating BIO"));
		return false;
	}

	BIO_write(bio, PrivateKey.Revert().get(), static_cast<int>(PrivateKey.length()));

	EVP_PKEY* pkey = nullptr;
	PEM_read_bio_PrivateKey(bio, &pkey, nullptr, nullptr);
	if (pkey != nullptr)
	{
		const auto privateKey = EVP_PKEY_get1_RSA(pkey);

		if (privateKey != nullptr)
		{
			const auto tmpDecSize = RSA_size((const RSA*)privateKey);
			auto  tmpDec = ALLOC<byte>(static_cast<size_t>(tmpDecSize) + 1);
			const auto  decLen = RSA_private_decrypt(static_cast<int>(size), *data, tmpDec, privateKey, RSA_PKCS1_OAEP_PADDING);
			if (decLen > 0)
			{
				FREE(*data);
				*data = tmpDec; // pointer swap
				data[0][decLen] = '\0';
				size = decLen;

				BIO_free(bio);
				RSA_free(privateKey);
				free(pkey);
				return true;
			}

			LOG(CSTRING("RSA failure: %s"), ERR_reason_error_string(ERR_get_error()));
			FREE(tmpDec);
			RSA_free(privateKey);
		}

		free(pkey);
	}

	BIO_free(bio);
	return false;
}

bool NETRSA::decryptStringHex(CryptoPP::byte** data, size_t& size)
{
	const NET_HEX hex;
	hex.decodeString(data, size);

	if (decryptString(data, size))
		return true;

	return false;
}

bool NETRSA::decryptStringBase64(CryptoPP::byte** data, size_t& size)
{
	NET_BASE64 base64;
	base64.decodeString(data, size);

	if (decryptString(data, size))
		return true;

	return false;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
