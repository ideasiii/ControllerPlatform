#include "AesCrypto.h"

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/config.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "LogHandler.h"

#define LOG_TAG "[AesCrypto]"

AesCrypto::AesCrypto(const uint8_t *key)
{
	this->key = key;
}

std::string AesCrypto::encrypt(const std::string plaintext, const uint8_t *iv)
{
	std::string ciphertext;
	CryptoPP::AES::Encryption aesEncryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	// Crypto++ will release the sink for us
	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(ciphertext));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size() + 1);
	stfEncryptor.MessageEnd();

	return ciphertext;
}

std::string AesCrypto::decrypt(const uint8_t *ciphertext, const int textLength, const uint8_t *iv)
{
	std::string decryptedtext;
	CryptoPP::AES::Decryption aesDecryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	try
	{
		// Crypto++ will release the sink for us
		CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
		stfDecryptor.Put(ciphertext, textLength);
		stfDecryptor.MessageEnd();
	}
	catch (CryptoPP::Exception const &e)
	{
		_log(LOG_TAG" decrypt failed: %s\n", e.what());
	}

	return decryptedtext;
}

void AesCrypto::getRandomBytes(uint8_t *outBuf, const int len)
{
	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, outBuf, len);
	close(fd);
}
