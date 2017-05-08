#include "Ites1fDacClient.h"

#include <errno.h>
#include <sstream>
#include <string.h>

#include "AesCrypto.h"
#include "FakeCmpClient.h"
#include "JSONObject.h"
#include "cJSON.h"


#define ENCRYPT_REQUEST_PDU_BODY
#define DECRYPT_RESPONSE_PDU_BODY

int encryptRequestBody(AesCrypto& crypto, const std::string& reqBody, uint8_t *oBuf, int bufSize);
std::string decryptRequestBody(AesCrypto& crypto, CMP_PACKET* pdu);

Ites1fDacClient::Ites1fDacClient(const char *ip, int port, const uint8_t *key):
	serverIp(ip), serverPort(port)
{
	// ITES_1F_DOOR_CONTROL_AES_KEY is defined in CryptoKey.h like this:
	// #define ITES_1F_DOOR_CONTROL_AES_KEY "a string with 32 characters....."
	
	//aesKey.assign(AesCrypto::KeyLength);
	aesKey.assign(key, key + AesCrypto::KeyLength);
}

Ites1fDacClient::~Ites1fDacClient()
{
}

bool Ites1fDacClient::doorOpen(std::string &errorDescription, std::string userUuid,
	std::string readerId, std::string token, int64_t validFrom, int64_t goodThrough)
{
	_log("[Ites1F_DAClient] Enter doorOpen()");
	CMP_PACKET reqPdu, respPdu;

	std::stringstream ss;
	ss << "{\"subject\": \"" << userUuid
		<< "\", \"verb\": \"doorOpen\""
		<< ", \"object\": \"" << readerId
		<< "\", \"validFrom\": " << validFrom
		<< ", \"validTo\": " << goodThrough
		<< "}";

	const std::string& reqBody = ss.str();
	_log("[Ites1F_DAClient] reqBody.size() = %d", reqBody.size());

#ifdef ENCRYPT_REQUEST_PDU_BODY
	AesCrypto crypto(aesKey.data());
	uint8_t buf[MAX_DATA_LEN];
	int bufSize = encryptRequestBody(crypto, reqBody, buf, sizeof(buf));

	if (bufSize < 1)
	{
		errorDescription = "Encrypting request body failed";
		return false;
	}
#else
	int bufSize = reqBody.size() + 1;
	uint8_t *buf = (uint8_t *)reqBody.c_str();
	_log("[Ites1F_DAClient] Not Encrypting request PDU body, bufSize = %d", bufSize);
#endif

	int reqPduSize = FakeCmpClient::craftCmpPdu(&reqPdu, bufSize, 
		smart_building_door_control_request, STATUS_ROK, 0, buf);

	if (reqPduSize < 1)
	{
		errorDescription = "Crafting request PDU failed";
		return false;
	}

	_log("[Ites1F_DAClient] reqPduSize = %d", reqPduSize);

	FakeCmpClient client(this->serverIp.data(), this->serverPort);
	int respPduSize = client.sendOnlyOneRequest(&reqPdu, reqPduSize, &respPdu);
	
	if (respPduSize < (int)sizeof(CMP_HEADER))
	{
		errorDescription.assign("Error sending request");
		return false;
	}

	
	// check return status in header

#ifdef DECRYPT_RESPONSE_PDU_BODY
	std::string respBodyStr = decryptRequestBody(crypto, &respPdu);

	if (respBodyStr.length() < 2)
	{
		_log("[Ites1F_DAClient] decrypt response body failed, raw response = %s", respPdu.cmpBody.cmpdata);
		errorDescription.assign("Reading server response failed");
		return false;
	}
	else
	{
		_log("[Ites1F_DAClient] response body decrypt ok: `%s`\n", respBodyStr.c_str());
	}
#else
	std::string respBodyStr(respPdu.cmpBody.cmpdata);
	_log("[Ites1F_DAClient] raw response: %s", (uint8_t*)respPdu.cmpBody.cmpdata);
#endif

	JSONObject respJson(respBodyStr);
	if (!respJson.isValid())
	{
		_log("[Ites1F_DAClient] respJson.isValid() = false");
		errorDescription.assign("Reading server response failed");
		return false;
	}

	bool success = respJson.getBoolean("success");
	if (!success)
	{
		bool granted = respJson.getBoolean("granted");
		std::string message = respJson.getString("message");

		_log("[Ites1F_DAClient] Request failed (granted: %s): `%s`\n",
			granted ? "true" : "false", message.c_str());
		
		if(granted)
		{
			errorDescription = "Permission granted by remote but action failed: " + message;
		}
		else
		{
			errorDescription = "Permission not granted by remote: " + message;
		}
		
		return false;
	}
	
	_log("[Ites1F_DAClient] Request successful\n");
	return true;
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
	AesCrypto::getRandomBytes(iv, AesCrypto::IvLength);

	std::string ciphertext = crypto.encrypt(reqBody, iv);
	int outputSize = AesCrypto::IvLength + ciphertext.size();

	if (outputSize > bufSize)
	{
		_log("[Ites1F_DAClient] Encrypt request PDU body failed, outputSize > bufSize (%d > %d)",
			outputSize, bufSize);
		return -1;
	}

	memcpy(oBuf, iv, AesCrypto::IvLength);
	memcpy(oBuf + AesCrypto::IvLength, ciphertext.c_str(), ciphertext.size());

	_log("[Ites1F_DAClient] Encrypt request PDU body, "
		"IV len = %d, reqBody.size() = %d, ciphertext.size() = %d, outputSize = %d"
		, AesCrypto::IvLength, reqBody.size(), ciphertext.size(), outputSize);

	return outputSize;
}

std::string decryptRequestBody(AesCrypto& crypto, CMP_PACKET* pdu)
{
	uint8_t *respIv = (uint8_t*)pdu->cmpBody.cmpdata;
	uint8_t *respBody = ((uint8_t*)pdu->cmpBody.cmpdata) + AesCrypto::IvLength;
	int respCipherLen = pdu->cmpHeader.command_length - sizeof(CMP_HEADER) - AesCrypto::IvLength;
	_log("[Ites1F_DAClient] response ciphertext length = %d", respCipherLen);

	std::string respBodyStr = crypto.decrypt(respBody, respCipherLen, respIv);
	_log("[Ites1F_DAClient] decrypt.length = %d", respBodyStr.length());

	return respBodyStr;
}
