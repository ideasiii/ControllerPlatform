#include "AesCrypto.h"

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/config.h>
#include "LogHandler.h"

#define LOG_TAG "[AesCrypto]"
#define MODE_CBC 0
#define MODE_CTR 1

AesCrypto *AesCrypto::createCbcInstance(const uint8_t *key)
{
	return new AesCrypto(MODE_CBC, key);
}

AesCrypto *AesCrypto::createCtrInstance(const uint8_t *key)
{
	return new AesCrypto(MODE_CTR, key);
}

AesCrypto::AesCrypto(int mode, const uint8_t *key)
	:mode(mode), key(key)
{
}

std::string AesCrypto::encrypt(const std::string plaintext, const uint8_t *iv)
{
	switch (mode)
	{
	case MODE_CBC:
		return encryptCbc(plaintext, iv);
	case MODE_CTR:
		return encryptCtr(plaintext, iv);
	default:
		return "";
	}
}

std::string AesCrypto::encryptCbc(const std::string plaintext, const uint8_t *iv)
{
	std::string ciphertext;
	CryptoPP::AES::Encryption aesEncryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption modeExternalCipher(aesEncryption, iv);

	// Crypto++ will release the sink for us
	CryptoPP::StreamTransformationFilter stfEncryptor(modeExternalCipher, new CryptoPP::StringSink(ciphertext));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size() + 1);
	stfEncryptor.MessageEnd();

	return ciphertext;
}

std::string AesCrypto::encryptCtr(const std::string plaintext, const uint8_t *iv)
{
	std::string ciphertext;

	try
	{
		CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption d;
		d.SetKeyWithIV(key, KeyLength, iv);

		// terminating zero of string will be encrypted as well
		CryptoPP::StringSource(
			(uint8_t *)plaintext.data(), plaintext.size() + 1, true,
			new CryptoPP::StreamTransformationFilter(
				d, new CryptoPP::StringSink(ciphertext)
			)
		);
	}
	catch (CryptoPP::Exception &e)
	{
		_log(LOG_TAG" encryptCtr() failed: %s\n", e.what());
	}

	return ciphertext;
}

std::string AesCrypto::decrypt(const uint8_t *cipher, const int length, const uint8_t *iv)
{
	switch (mode)
	{
	case MODE_CBC:
		return decryptCbc(cipher, length, iv);
	case MODE_CTR:
		return decryptCtr(cipher, length, iv);
	default:
		return "";
	}
}

std::string AesCrypto::decryptCbc(const uint8_t *cipher, const int length, const uint8_t *iv)
{
	std::string decryptedtext;
	CryptoPP::AES::Decryption aesDecryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	try
	{
		// Crypto++ will release the sink for us
		CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
		stfDecryptor.Put(cipher, length);
		stfDecryptor.MessageEnd();
	}
	catch (CryptoPP::Exception &e)
	{
		_log(LOG_TAG" decryptCbc() failed: %s\n", e.what());
	}

	return decryptedtext;
}

std::string AesCrypto::decryptCtr(const uint8_t *cipher, const int length, const uint8_t *iv)
{
    std::string decryptedText;

    try
    {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption d;
        d.SetKeyWithIV(key, KeyLength, iv);

        CryptoPP::StringSource(
            cipher, length, true,
            new CryptoPP::StreamTransformationFilter(
                d, new CryptoPP::StringSink(decryptedText)
            )
        );
    }
    catch(CryptoPP::Exception &e)
    {
        _log(LOG_TAG" decryptCtr() failed: %s\n", e.what());
    }

    return decryptedText;
}

void AesCrypto::splitIvAndCipher(const uint8_t *src, uint srcLen, uint8_t **outIv, uint8_t **outCipher)
{
	if (srcLen < CryptoPP::AES::BLOCKSIZE)
	{
		*outIv = NULL;
		*outCipher = NULL;
		return;
	}

	*outIv = new uint8_t[CryptoPP::AES::BLOCKSIZE];
	memcpy(*outIv, src, CryptoPP::AES::BLOCKSIZE);

	//printf("iv (%lu): ", sizeof(iv));
	//arrayToOneLineHex(iv, sizeof(iv));

	uint cipherLen = srcLen-CryptoPP::AES::BLOCKSIZE;
	if (cipherLen == 0)
	{
		*outCipher = NULL;
	}
	else
	{
	    *outCipher = new byte[cipherLen];
	    memcpy(*outCipher, src+CryptoPP::AES::BLOCKSIZE, cipherLen);

	    //printf("cipherRaw (%u): ", cipherLen);
		//arrayToOneLineHex(cipherRaw, cipherLen);
	}
}
