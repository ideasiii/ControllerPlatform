#include "Ites1fDacClient.h"

#include <errno.h>
#include <memory>
#include <sstream>
#include <string.h>
#include "AesCrypto.h"
#include "FakeCmpClient.h"
#include "../HiddenUtility.hpp"
#include "JSONObject.h"

#define ENCRYPT_REQUEST_PDU_BODY
#define DECRYPT_RESPONSE_PDU_BODY

#define LOG_TAG "[Ites1fDacClient]"

int encryptRequestBody(AesCrypto& crypto, const std::string& reqBody, uint8_t *oBuf, int bufSize);
std::string decryptRequestBody(AesCrypto& crypto, CMP_PACKET* pdu);

Ites1fDacClient::Ites1fDacClient(const std::string ip, int port, const uint8_t *key):
	serverIp(ip), serverPort(port)
{
	_log(LOG_TAG" construct with server ip %s, port %d. key not shown.", ip.c_str(), port);

	memcpy(aesKey, key, AesCrypto::KeyLength);
}

Ites1fDacClient::~Ites1fDacClient()
{
}

bool Ites1fDacClient::doorOpen(std::string &errorDescription, std::string userUuid,
	std::string readerId, std::string token, int64_t validFrom, int64_t goodThrough)
{
	_log(LOG_TAG" doorOpen() step in");
	CMP_PACKET reqPdu, respPdu;

	std::stringstream ss;
	ss << "{\"subject\": \"" << userUuid
		<< "\", \"verb\": \"doorOpen\""
		<< ", \"object\": \"" << readerId
		<< "\", \"validFrom\": " << validFrom
		<< ", \"validTo\": " << goodThrough
		<< "}";

	const std::string& reqBody = ss.str();
	_log(LOG_TAG" doorOpen() reqBody.size() = %d", reqBody.size());

#ifdef ENCRYPT_REQUEST_PDU_BODY
	unique_ptr<AesCrypto> crypto(AesCrypto::createCbcInstance(aesKey));
	uint8_t buf[MAX_DATA_LEN];

	// 加密前的字串須以 '\0' 結尾，但加密後的內容不須以 '\0' 結尾
	// 如果字串後面要加 padding，在加密之前就必須附加於其後，且中間至少以一個 '\0' 隔開
	int bufSize = encryptRequestBody(*(crypto.get()), reqBody, buf, sizeof(buf));

	if (bufSize < 1)
	{
		errorDescription = "Request body encryption failed";
		return false;
	}
#else
	int bufSize = reqBody.size() + 1;
	uint8_t *buf = (uint8_t *)reqBody.c_str();
	_log(LOG_TAG" Not Encrypting request PDU body, bufSize = %d", bufSize);
#endif

	int reqPduSize = FakeCmpClient::craftCmpPdu(&reqPdu, bufSize,
		smart_building_door_control_request, STATUS_ROK, 0, buf);

	if (reqPduSize < 1)
	{
		errorDescription = "Crafting request PDU failed";
		return false;
	}

	_log(LOG_TAG" doorOpen() reqPduSize = %d", reqPduSize);

	FakeCmpClient client(this->serverIp.data(), this->serverPort);
	int respPduSize = client.sendOnlyOneRequest(&reqPdu, reqPduSize, &respPdu);

	if (respPduSize < (int)sizeof(CMP_HEADER))
	{
		errorDescription = "Error sending request";
		return false;
	}

#ifdef DECRYPT_RESPONSE_PDU_BODY
	std::string respBodyStr = decryptRequestBody(*(crypto.get()), &respPdu);

	if (respBodyStr.length() < 2)
	{
		_log(LOG_TAG" doorOpen() decrypt response body failed, raw response = %s", respPdu.cmpBody.cmpdata);
		errorDescription = "Reading server response failed";
		return false;
	}
	else
	{
		_log(LOG_TAG" doorOpen() response body decrypt ok: `%s`\n", respBodyStr.c_str());
	}
#else
	std::string respBodyStr(respPdu.cmpBody.cmpdata);
	_log(LOG_TAG" raw response: %s", (uint8_t*)respPdu.cmpBody.cmpdata);
#endif

	JSONObject respJson(respBodyStr);
	if (!respJson.isValid())
	{
		_log(LOG_TAG" doorOpen() respJson.isValid() == false");
		errorDescription = "Server failed to response";
		return false;
	}

	bool success = respJson.getBoolean("success");
	if (!success)
	{
		bool granted = respJson.getBoolean("granted");
		std::string message = respJson.getString("message");

		_log(LOG_TAG" doorOpen() request failed (granted: %s): `%s`\n",
			granted ? "true" : "false", message.c_str());

		if(granted)
		{
			errorDescription = "Permission granted but action failed: " + message;
		}
		else
		{
			errorDescription = "Permission not granted: " + message;
		}
	}
	else
	{
		_log(LOG_TAG" doorOpen() request successful\n");
	}

	respJson.release();
	return success;
}

/**
* 加密 reqBody
* @param oBuf 儲存加密結果的指針，資料的格式為 IV接加密後的reqBody，中間沒有間隔
* @return 加密後的資料大小
*/
int encryptRequestBody(AesCrypto& crypto, const std::string& reqBody,
	uint8_t *oBuf, int bufSize)
{
	uint8_t iv[AesCrypto::IvLength];
	HiddenUtility::getRandomBytes(iv, AesCrypto::IvLength);

	std::string ciphertext = crypto.encrypt(reqBody, iv);
	int outputSize = AesCrypto::IvLength + ciphertext.size();

	if (outputSize > bufSize)
	{
		_log(LOG_TAG" Encrypt request PDU body failed, outputSize > bufSize (%d > %d)",
			outputSize, bufSize);
		return -1;
	}

	memcpy(oBuf, iv, AesCrypto::IvLength);
	memcpy(oBuf + AesCrypto::IvLength, ciphertext.c_str(), ciphertext.size());

	_log(LOG_TAG" Encrypt request PDU body, "
		"IV len = %d, reqBody.size() = %d, ciphertext.size() = %d, outputSize = %d"
		, AesCrypto::IvLength, reqBody.size(), ciphertext.size(), outputSize);

	return outputSize;
}

std::string decryptRequestBody(AesCrypto& crypto, CMP_PACKET* pdu)
{
	int respCipherLen = pdu->cmpHeader.command_length - sizeof(CMP_HEADER) - AesCrypto::IvLength;
	if (respCipherLen < 0)
	{
		_log(LOG_TAG" decryptRequestBody() response ciphertext length invalid (%d)", respCipherLen);
		return "";
	}

	_log(LOG_TAG" decryptRequestBody() response ciphertext length = %d", respCipherLen);

	uint8_t *respIv = (uint8_t*)pdu->cmpBody.cmpdata;
	uint8_t *respBody = ((uint8_t*)pdu->cmpBody.cmpdata) + AesCrypto::IvLength;

	std::string respBodyStr = crypto.decrypt(respBody, respCipherLen, respIv);
	_log(LOG_TAG" decryptRequestBody() decrypt.length = %d", respBodyStr.length());

	return respBodyStr;
}
