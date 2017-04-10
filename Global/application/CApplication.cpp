/*
 * CApplication.cpp
 *
 *  Created on: 2017年4月8日
 *      Author: root
 */

#include <syslog.h>
#include <string>
#include "CApplication.h"
#include "CController.h"
#include "CMessageHandler.h"
#include "CProcessHandler.h"
#include "common.h"
#include "CConfig.h"
#include "Message.h"
#include "event.h"

using namespace std;
using namespace apmsg;
extern char *__progname;

int ClientReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

int ServerReceive(int nSocketFD, int nDataLen, const void *pData)
{
	return 0;
}

CApplication::CApplication()
{
	mapFunc[MSG_ON_INITIAL] = &CApplication::onInitial;
	mapFunc[MSG_ON_FINISH] = &CApplication::onFinish;
}

CApplication::~CApplication()
{

}

void CApplication::callback(int nMsg)
{
	map<int, MemFn>::iterator iter;
	if(mapFunc.end() != (iter = mapFunc.find(nMsg)))
	{
		(this->*this->mapFunc[nMsg])();
	}
}

void CApplication::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{

}

void CApplication::setConfPath(const char * szPath)
{
	mstrConfPath = szPath;
}
string CApplication::getConfPath()
{
	return mstrConfPath;
}

inline string getConfName(std::string strProcessName)
{
	size_t found = strProcessName.find_last_of("/\\");
	return (strProcessName.substr(++found) + ".conf");
}
inline void initLogPath()
{
	string strPath;
	CConfig *config = new CConfig();
	string *pstrConf = new string(getConfName(__progname));
	if(config->loadConfig(*pstrConf))
	{
		strPath = config->getValue("LOG", "log");
		if(!strPath.empty())
		{
			_setLogPath(strPath.c_str());
		}
		else
		{
			_setLogPath("./controller.log");
		}
	}
	delete pstrConf;
	delete config;
}

void runService()
{
	int nInit = FALSE;
	int nTmp = -1;
	string strConfPath;

	openlog(__progname, LOG_PID, LOG_LOCAL0);
	CController *controller = new CController();
	if(0 < controller->initMessage(clock(), "Controller"))
	{
		initLogPath();
		_log("\n<============= (◕‿‿◕｡) ... Service Start Run ... ԅ(¯﹃¯ԅ) =============>\n");
		controller->setConfPath(getConfName(__progname).c_str());
		controller->callback(MSG_ON_INITIAL);
		controller->run(EVENT_FILTER_CONTROLLER, "Controller");
		controller->callback(MSG_ON_FINISH);
		CMessageHandler::release();
	}
	_close(); // close log file
	delete controller;
	_log("\n<============= ( #｀Д´) ... Service Stop Run ... (╬ ಠ 益ಠ) =============>\n");
	closelog();
}

int main(int argc, char* argv[])
{
	return process(runService);
}
