#include "AesCrypto.h"

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/config.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


AesCrypto::AesCrypto(uint8_t *key)
{
	this->key = key;
}

std::string AesCrypto::encrypt(std::string plaintext, uint8_t *iv)
{
	std::string ciphertext;
	CryptoPP::AES::Encryption aesEncryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(ciphertext));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.size() + 1);
	stfEncryptor.MessageEnd();

	return ciphertext;
}

std::string AesCrypto::decrypt(unsigned char *ciphertext, int textLength, uint8_t *iv)
{
	std::string decryptedtext;
	CryptoPP::AES::Decryption aesDecryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	try
	{
		CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
		stfDecryptor.Put(ciphertext, textLength);
		stfDecryptor.MessageEnd();
	}
	catch (CryptoPP::Exception const &e)
	{
		// TODO make visible outside
		printf("decrypt failed: %s\n", e.what());
		//decryptedtext = std::string();
	}
	
	return decryptedtext;
}

void AesCrypto::getRandomBytes(uint8_t *outBuf, int bufLen)
{
	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, outBuf, bufLen);
	close(fd);
}
