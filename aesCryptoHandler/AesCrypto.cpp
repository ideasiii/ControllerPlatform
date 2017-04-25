#include "AesCrypto.h"

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

AesCrypto::AesCrypto(byte *key) 	
{
	this->key = key;
}

std::string AesCrypto::encrypt(std::string plaintext, byte *iv)
{
	std::string ciphertext;
	CryptoPP::AES::Encryption aesEncryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

	CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(ciphertext));
	stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length() + 1);
	stfEncryptor.MessageEnd();

	return ciphertext;
}

std::string AesCrypto::decrypt(unsigned char *ciphertext, int textLength, byte *iv)
{
	std::string decryptedtext;
	CryptoPP::AES::Decryption aesDecryption(key, KeyLength);
	CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

	CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
	stfDecryptor.Put(ciphertext, textLength);
	stfDecryptor.MessageEnd();

	return decryptedtext;
}

void AesCrypto::getRandomBytes(byte *outBuf, int bufLen)
{
	int fd = open("/dev/urandom", O_RDONLY);
	read(fd, outBuf, bufLen);
}
