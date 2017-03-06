/*
 * CServerCenter.cpp
 *
 *  Created on: 2016-12-05
 *      Author: Jugo
 */

#include <map>
#include "event.h"
#include "packet.h"
#include "common.h"
#include "CCmpHandler.h"
#include "CDataHandler.cpp"
#include "JSONObject.h"
#include "JSONArray.h"
#include "CServerCenter.h"
#include "ICallback.h"
#include "CSqliteHandler.h"
#include "IReceiver.h"

using namespace std;

static CServerCenter * serverCenter = 0;

#define DB_PATH_IDEAS				"/data/sqlite/ideas.db"
#define RESP_INIT "{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"175.98.119.121\",\"port\": 6607	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"175.98.119.121\",\"port\": 2307}]}"

typedef struct _SIGNUP_DATA
{
	string id;
	string app_id;
	string mac;
	string os;
	string phone;
	string fb_id;
	string fb_name;
	string fb_email;
	string fb_account;
	string g_account;
	string t_account;
} SIGNUP_DATA;

CServerCenter::CServerCenter() :
		CSocketServer(), cmpParser(CCmpHandler::getInstance())
{
	mapFunc[bind_request] = &CServerCenter::cmpBind;
	mapFunc[unbind_request] = &CServerCenter::cmpUnbind;
	mapFunc[initial_request] = &CServerCenter::cmpInitial;
	mapFunc[sign_up_request] = &CServerCenter::cmpSignup;
}

CServerCenter::~CServerCenter()
{
	stop();
	mapClient.clear();
}

CServerCenter * CServerCenter::getInstance()
{
	if(0 == serverCenter)
	{
		serverCenter = new CServerCenter();
	}
	return serverCenter;
}

int CServerCenter::startServer(const char *szIP, const int nPort, const int nMsqId)
{
	if(0 >= nPort || 0 >= nMsqId)
		return FALSE;

	/** Run socket server for CMP **/
	if(0 < nMsqId)
	{
		setPackageReceiver(nMsqId, EVENT_FILTER_CONTROLLER, EVENT_COMMAND_SOCKET_TCP_CENTER_RECEIVE);
		setClientConnectCommand(EVENT_COMMAND_SOCKET_CLIENT_CONNECT_CENTER);
		setClientDisconnectCommand(EVENT_COMMAND_SOCKET_CLIENT_DISCONNECT_CENTER);
	}

	/** Set Receive , Packet is CMP , Message Queue Handle **/
	setPacketConf(PK_CMP, PK_MSQ);

	if(FAIL == start(AF_INET, szIP, nPort))
	{
		_log("[Server Center] Socket Create Fail");
		return FALSE;
	}

	return TRUE;
}

void CServerCenter::stopServer()
{
	stop();
}

void CServerCenter::onReceive(const int nSocketFD, const void *pData)
{
	int nRet = -1;
	int nPacketLen = 0;
	CMP_HEADER cmpHeader;
	char *pPacket;

	pPacket = (char*) const_cast<void*>(pData);
	memset(&cmpHeader, 0, sizeof(CMP_HEADER));

	cmpHeader.command_id = cmpParser->getCommand(pPacket);
	cmpHeader.command_length = cmpParser->getLength(pPacket);
	cmpHeader.command_status = cmpParser->getStatus(pPacket);
	cmpHeader.sequence_number = cmpParser->getSequence(pPacket);

	printPacket(cmpHeader.command_id, cmpHeader.command_status, cmpHeader.sequence_number, cmpHeader.command_length,
			"[Server Center] Recv ", nSocketFD);

	if(cmpParser->isAckPacket(cmpHeader.command_id))
	{
		return;
	}

	map<int, MemFn>::iterator iter;
	iter = mapFunc.find(cmpHeader.command_id);

	if(0x000000FF < cmpHeader.command_id || 0x00000000 >= cmpHeader.command_id || mapFunc.end() == iter)
	{
		sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocketFD, generic_nack | cmpHeader.command_id,
		STATUS_RINVCMDID, cmpHeader.sequence_number, 0);
		return;
	}

	(this->*this->mapFunc[cmpHeader.command_id])(nSocketFD, cmpHeader.command_id, cmpHeader.sequence_number, pPacket);

}

int CServerCenter::cmpBind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	mapClient[nSocket] = nSocket;
	_log("[Server Center] Socket Client FD:%d Binded", nSocket);
	sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}

int CServerCenter::cmpUnbind(int nSocket, int nCommand, int nSequence, const void *pData)
{
	if(mapClient.end() != mapClient.find(nSocket))
	{
		mapClient.erase(nSocket);
		_log("[Server Center] Socket Client FD:%d Unbinded", nSocket);
	}
	sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence, 0);
	return TRUE;
}

void CServerCenter::setCallback(const int nId, CBFun cbfun)
{
	mapCallback[nId] = cbfun;
}

void CServerCenter::addClient(const int nSocketFD)
{
	_log("[Server Center] Socket Client FD:%d Connected", nSocketFD);
}

void CServerCenter::deleteClient(const int nSocketFD)
{
	if(mapClient.end() != mapClient.find(nSocketFD))
	{
		mapClient.erase(nSocketFD);
		_log("[Server Center] Socket Client FD:%d Unbinded", nSocketFD);
	}
	_log("[Server Center] Socket Client FD:%d Disconnected", nSocketFD);
}

int CServerCenter::cmpInitial(int nSocket, int nCommand, int nSequence, const void *pData)
{
	char *pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
	int nType = ntohl(*((int*) pBody));
	_log("[Server Center] Receice CMP Init: type=%d ", nType);

	switch(nType)
	{
	case TYPE_MOBILE_SERVICE:
	case TYPE_POWER_CHARGE_SERVICE:
	case TYPE_SDK_SERVICE:
	case TYPE_TRACKER_SERVICE:
	case TYPE_TRACKER_APPLIENCE:
	case TYPE_TRACKER_TOY:
	case TYPE_TRACKER_IOT:
		sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_ROK, nSequence,
		RESP_INIT);
		return 0;

	}

	_log("[Server Center] Initial Fail, Can't get initial data Socket FD:%d, Service Type: %d", nSocket, nType);
	sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_RSYSERR, nSequence, 0);

	return 0;
}

int CServerCenter::cmpSignup(int nSocket, int nCommand, int nSequence, const void *pData)
{
	CSqliteHandler sqlite;
	char *pBody = (char*) ((char *) const_cast<void*>(pData) + sizeof(CMP_HEADER));
	int nType = ntohl(*((int*) pBody));
	pBody += 4;

	if(!sqlite.connectDB(DB_PATH_IDEAS))
	{
		_log("[CServerCenter] Error: Sqlite Connect Fail");
		return sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_RSYSERR,
				nSequence, 0);;
	}

	if(isValidStr((const char*) pBody, MAX_SIZE))
	{
		char temp[MAX_SIZE];
		memset(temp, 0, sizeof(temp));
		strcpy(temp, pBody);
		if(0 < strlen(temp))
		{
			JSONObject jsonData(temp);
			if(jsonData.isValid())
			{
				SIGNUP_DATA signupData;
				signupData.id = jsonData.getString("id");
				if(!signupData.id.empty())
				{
					signupData.app_id = jsonData.getString("app_id");
					signupData.fb_account = jsonData.getString("fb_account");
					signupData.fb_email = jsonData.getString("fb_email");
					signupData.fb_id = jsonData.getString("fb_id");
					signupData.fb_name = jsonData.getString("fb_name");
					signupData.g_account = jsonData.getString("g_account");
					signupData.mac = jsonData.getString("mac");
					signupData.os = jsonData.getString("os");
					signupData.phone = jsonData.getString("phone");
					signupData.t_account = jsonData.getString("t_account");

					/** Check ID exist in user Table. **/
					string strSQL = "SELECT id FROM user WHERE id = '" + signupData.id + "';";
					//	list<string> listValue;
					//	int nRow = sqlite->ideasSqlExec(strSQL.c_str(), listValue, 0);

					JSONArray jsonArray;
					sqlite.query(strSQL, jsonArray);

					if(0 >= jsonArray.size())
					{
						strSQL =
								"INSERT INTO user(id,app_id,mac,os,phone,fb_id,fb_name,fb_email,fb_account,g_account,t_account) VALUES('"
										+ signupData.id + "','" + signupData.app_id + "','" + signupData.mac + "','"
										+ signupData.os + "','" + signupData.phone + "','" + signupData.fb_id + "','"
										+ signupData.fb_name + "','" + signupData.fb_email + "','"
										+ signupData.fb_account + "','" + signupData.g_account + "','"
										+ signupData.t_account + "');";
						if( TRUE == sqlite.sqlExec(strSQL.c_str()))
						{
							_log("[Server Center] Add User Login Data Success.");
						}
						else
							return sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand,
							STATUS_RSYSERR, nSequence, 0);

					}
					_log(
							"[Server Center] Signup Request:id=%s app_id=%s fb_account=%s fb_email=%s fb_id=%s fb_name=%s g_account=%s mac=%s os=%s phone=%s t_account=%s",
							signupData.id.c_str(), signupData.app_id.c_str(), signupData.fb_account.c_str(),
							signupData.fb_email.c_str(), signupData.fb_id.c_str(), signupData.fb_name.c_str(),
							signupData.g_account.c_str(), signupData.mac.c_str(), signupData.os.c_str(),
							signupData.phone.c_str(), signupData.t_account.c_str());
					return sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand,
					STATUS_ROK, nSequence, 0);
				}

			}
			jsonData.release();
		}
	}

	_log("[Server Center] Signup Fail, Can't get Signup data Socket FD:%d, Service Type: %d", nSocket, nType);
	return sendPacket(dynamic_cast<CSocket*>(serverCenter), nSocket, generic_nack | nCommand, STATUS_RSYSERR, nSequence,
			0);

}

