#pragma once

#include "packet.h"

/**
* 與「偽﹒CMP」(Fake CMP) server 溝通的客戶端
* Fake CMP 與一般 CMP 的差異：
*   - client 不先送 bind request, server 把 bind request 當亂碼
*   - client 一連上 server 就直接送要做的命令
*   - server 在處理完第一通 request 後就主動斷線
*   - PDU body 不一定以 '\0' 做結尾
*
* Fake CMP client & server 通訊流程
*   - client 連線至 server 
*   - 建立連線後，client 送出一個 PDU
*   - server 回應一個 PDU，然後斷線
*/
class FakeCmpClient
{
public:
	FakeCmpClient(char *ip, int port);
	~FakeCmpClient();

	/**
	* 發送一個請求到 Fake CMP server
	* @param  requestPdu  要傳送的 PDU，body 長度不可超過 MAX_DATA_LEN，header 需先轉換為 network byte order
	* @param  responsePdu 存放接收的 PDU 指針，header 會轉換為 host byte order
	* @return             how many bytes received (PDU length), <0 on error.
	*/
	int sendOnlyOneRequest(CMP_PACKET *requestPdu, int reqPduLen, CMP_PACKET *responsePdu, char **errorDescription);
	
	/**
	* helper function for building request PDU
	* @param dst        Pointer to output PDU
	* @param bodyLength Length of szData
	* @param szData     Body, size must not exceed (MAX_DATA_LEN - sizeof(CMP_HEADER))
	* @return           If crafting process is ok, return the size of PDU, otherwise -1
	*/
	static int craftCmpPdu(CMP_PACKET *dst, int bodyLength, const int nCommandId,
		const int nStatus, const int nSequence, const uint8_t *szData);

private:
	char *serverIp;
	int serverPort;
};
