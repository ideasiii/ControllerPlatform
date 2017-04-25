#pragma once

/**
 * A helper class for doing AES256, CBC mode encryption/decryption.
 * 
 * Compilation requires crypto++, you can install it through apt on Debian:
 *     sudo apt-get -y install libcrypto++-dev
 * Linking option: -lcryptopp
 */
class AesCrypto
{
public:
	/**
	 * Byte length of initial vector.
	 */
	static const int IvLength = 16;

	/**
	 * Byte length of key, not including terminating null. 
	 * Note that 32 bytes = 256 bit.
	 */
	static const int KeyLength = 32; 

	/**
	 * Instantiate with a key used for both
	 * encryption and decryption process.
	 */
	AesCrypto(byte *key);

	std::string encrypt(std::string plaintext, byte *iv);
	std::string decrypt(unsigned char* ciphertext, int textLength, byte *iv);

	/**
	 * A handy function for generating random IV or (probably not) key.
	 */
	static void getRandomBytes(byte *outBuf, int bufLen);

private:
	byte *key;
};

/** example of usage, adapted from official Crypto++ example using CBC mode

#include <string>
void cryptoppTest() {
	// Byte length of key must be 32 bytes (not including teminating null)
	std::string key = "12345678901234567890123456789012";
	byte *key = (byte *)key.c_str();

	// Initial vector can be a array filled with zero or constant value
	// but it raises security concern, not recommended
	byte *iv = new byte[AesCrypto::IvLength];

	auto aes = AesCrypto(key);
	AesCrypto::getRandomBytes(iv, AesCrypto::IvLength);

	//
	// String and Sink setup
	//
	std::string plaintext = "{\"verb\":\"open\",\"subject\":\"yooooo\",\"object\":\"001\"}";
	std::string ciphertext;
	std::string decryptedtext;

	//
	// Dump Plain Text
	//
	std::cout << "Plain Text (" << plaintext.size() << " bytes)" << std::endl;
	std::cout << plaintext;
	std::cout << std::endl << std::endl;

	//
	// Create Cipher Text
	//
	ciphertext = std::string(aes.encrypt(plaintext, iv));

	//
	// Dump Cipher Text
	//
	std::cout << "Cipher Text (" << ciphertext.size() << " bytes)" << std::endl;

	for (unsigned int i = 0; i < ciphertext.size(); i++) {
	std::cout << "0x" << std::hex << (0xFF & static_cast<byte>(ciphertext[i])) << " ";
	}

	std::cout << std::endl << std::endl;

	//
	// Decrypt
	//
	decryptedtext = std::string(aes.decrypt((unsigned char*)(ciphertext.c_str()),
	ciphertext.size(), iv));


	//
	// Dump Decrypted Text
	//
	std::cout << "Decrypted Text: " << std::endl;
	std::cout << decryptedtext;
	std::cout << std::endl << std::endl;
}
*/