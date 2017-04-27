#include "Ites1fDoorAccessControlHandler.h"

#include "AesCrypto.h"
#include "CryptoKey.h"
#include "FakeCmpClient.h"
#include <string.h>
#include <sstream>
#include "packet.h"
#include <errno.h>

#define ENCRYPT_REQUEST_PDU_BODY
#define DECRYPT_RESPONSE_PDU_BODY

Ites1fDoorAccessControlHandler::Ites1fDoorAccessControlHandler(char *ip, int port):
	serverIp(ip), serverPort(port), aesKey((uint8_t*)ITES_1F_DOOR_CONTROL_AES_KEY)
{
	// ITES_1F_DOOR_CONTROL_AES_KEY is defined in CryptoKey.h like this:
	// #define ITES_1F_DOOR_CONTROL_AES_KEY "a string with length of 32"

	// ITES_1F_DOOR_CONTROL_AES_KEY byte array size is actually 33 because of 
	// terminating null appended at the end, but don't worry about it as it 
	// will be truncated by AesCrypto.
}

Ites1fDoorAccessControlHandler::~Ites1fDoorAccessControlHandler()
{
}

bool Ites1fDoorAccessControlHandler::doorOpen(std::string &errorDescription, std::string userUuid,
	std::string readerId, std::string token, int64_t validFrom, int64_t goodThrough)
{
	_log("doorOpen");
	CMP_PACKET reqPdu, respPdu;

	std::stringstream ss;
	ss << "{\"subject\": \"" << userUuid
		<< "\", \"verb\": \"doorOpen\""
		<< ", \"object\": \"" << readerId
		<< "\", \"validFrom\": " << validFrom
		<< ", \"validTo\": " << goodThrough
		<< "}";

	std::string reqBody = ss.str();
	_log("reqBody.size() = %d", reqBody.size());

	// encrypt
	
#ifdef ENCRYPT_REQUEST_PDU_BODY
	AesCrypto crypto(aesKey);
	uint8_t iv[AesCrypto::IvLength];
	AesCrypto::getRandomBytes(iv, AesCrypto::IvLength);

	std::string ciphertext = crypto.encrypt(reqBody, iv);
	int bufSize = AesCrypto::IvLength + ciphertext.size();
	uint8_t *buf = new uint8_t[bufSize];

	memcpy(buf, iv, AesCrypto::IvLength);
	memcpy(buf + AesCrypto::IvLength, ciphertext.c_str(), ciphertext.size());

	_log("[Ites1F_DACHandler] Encrypt request PDU body, IV len = %d, ciphertext.size() = %d, bufSize = %d"
		, AesCrypto::IvLength, ciphertext.size(), bufSize);
#else
	int bufSize = reqBody.size() + 1;
	uint8_t *buf = reqBody.c_str();
	_log("[Ites1F_DACHandler] Not Encrypting request PDU body, bufSize = %d", bufSize);
#endif

	int reqPduSize = FakeCmpClient::craftCmpPdu(&reqPdu, bufSize, 
		smart_building_door_control_request, STATUS_ROK, 0, buf);

#ifndef ENCRYPT_REQUEST_PDU_BODY
	delete[] buf;
#endif

	if (reqPduSize < 1)
	{
		errorDescription = "Crafting request PDU failed";
		return false;
	}

	_log("[Ites1F_DACHandler] reqPduSize = %d", reqPduSize);

	FakeCmpClient client(this->serverIp, this->serverPort);
	char *sendErrDesc = nullptr;
	int respPduSize = client.sendOnlyOneRequest(&reqPdu, reqPduSize, &respPdu, &sendErrDesc);
	int cmpHeaderSize = sizeof(CMP_HEADER);

	if (respPduSize < cmpHeaderSize)
	{
		errorDescription.assign(sendErrDesc);
		return false;
	}

	// check return status in header
	// decrypt first then remove padding (if appended)

#ifdef DECRYPT_RESPONSE_PDU_BODY
	uint8_t *respIv = (uint8_t*)respPdu.cmpBody.cmpdata;
	uint8_t *respBody = ((uint8_t*)respPdu.cmpBody.cmpdata) + AesCrypto::IvLength;
	int respCipherLen = respPdu.cmpHeader.command_length - sizeof(CMP_HEADER) - AesCrypto::IvLength;
	_log("[Ites1F_DACHandler] response ciphertext length = %d", respCipherLen);

	std::string respBodyStr = crypto.decrypt(respBody, respCipherLen, respIv);
	//_log("[Ites1F_DACHandler] decrypt.length = %d", respBodyStr.length());

	if (respBodyStr.length() < 1)
	{
		_log("[Ites1F_DACHandler] decrypt response body failed, raw response = %s", respIv);
		return false;
	}
	else
	{
		_log("[Ites1F_DACHandler] response body decrypt ok: `%s`\n", respBodyStr.c_str());
		return false;
	}
#else
	std::string respBodyStr(respPdu.cmpBody.cmpdata);
	_log("[Ites1F_DACHandler] raw response: %s", (uint8_t*)respPdu.cmpBody.cmpdata);
#endif

	// parse json
	//respBodyStr

	return true;
}
