/*
 * CDispatcher.cpp
 *
 *  Created on: 2017年3月7日
 *      Author: Jugo
 */

#include "CDispatcher.h"
#include "common.h"
#include "LogHandler.h"
#include "packet.h"
#include "utility.h"
#include <string>
#include <sys/prctl.h>
#include <unistd.h>
#include <signal.h>
#include "JSONObject.h"
#include "JSONArray.h"

using namespace std;

//static const char* RESP_DISPATCH =
//		"{\"server\": [{\"id\": 0,\"name\": \"startTrack\",\"ip\": \"54.199.198.94\",\"port\": 2306	},	{\"id\": 1,\"name\": \"tracker\",\"ip\": \"54.199.198.94\",\"port\": 2307}]}";

static CDispatcher * dispatcher = 0;

CDispatcher* CDispatcher::getInstance()
{
	if(0 == dispatcher)
	{
		dispatcher = new CDispatcher();
	}
	return dispatcher;
}

CDispatcher::CDispatcher()
{

}

CDispatcher::~CDispatcher()
{
	mapServer.clear();
}

int CDispatcher::onInitial(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	return response(nSocket, nCommand, STATUS_ROK, nSequence, mstrResp.c_str());
}

int CDispatcher::onDie(int nSocket, int nCommand, int nSequence, const void *szBody)
{
	_log("[CDispatcher] onDie .....%s", szBody);
	response(nSocket, nCommand, STATUS_ROK, nSequence, 0);
	kill(getpid(), SIGTERM);
	kill(getppid(), SIGTERM);

	return 0;
}

CDispatcher & CDispatcher::addServer(int nId, const char *szName, const char *szIp, int nPort)
{
	SERVER server;
	server.nPort = nPort;
	server.strIp = szIp;
	server.strName = szName;
	mapServer[nId] = server;
	return (*this);
}

void CDispatcher::createResp()
{
	JSONObject jsonroot;
	JSONArray jsonArry;
	JSONObject jsonItem;

	jsonroot.create();
	jsonArry.create();

	if(mapServer.end() != mapServer.find(ID_SERVER_SIGNIN))
	{
		jsonItem.create();
		jsonItem.put("id", ID_SERVER_SIGNIN);
		jsonItem.put("name", mapServer[ID_SERVER_SIGNIN].strName);
		jsonItem.put("ip", mapServer[ID_SERVER_SIGNIN].strIp);
		jsonItem.put("port", mapServer[ID_SERVER_SIGNIN].nPort);
		jsonArry.add(jsonItem);
		jsonItem.release();
	}

	if(mapServer.end() != mapServer.find(ID_SERVER_TRACKER))
	{
		jsonItem.create();
		jsonItem.put("id", ID_SERVER_TRACKER);
		jsonItem.put("name", mapServer[ID_SERVER_TRACKER].strName);
		jsonItem.put("ip", mapServer[ID_SERVER_TRACKER].strIp);
		jsonItem.put("port", mapServer[ID_SERVER_TRACKER].nPort);
		jsonArry.add(jsonItem);
		jsonItem.release();
	}

	jsonroot.put("server", jsonArry);
	mstrResp = trim(jsonroot.toJSON());
	jsonArry.release();
	jsonroot.release();

	_log("[CDispatcher] createResp : %s", mstrResp.c_str());
}
