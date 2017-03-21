#pragma once

#include <string>
#include <map>

#include "CCmpServer.h"

#include "iCommand.h"
#include "ICallback.h"

using namespace std;

class CServerAccessLog: public CCmpServer
{

public:

	CServerAccessLog();
	virtual ~CServerAccessLog();
	int onAccesslog(int nSocket, int nCommand, int nSequence, const void *pData);

void setCallback(const int nId, CBFun cbfun);

protected:

private:

map<int, CBFun> mapCallback;
int paseBody(const void *pData, CDataHandler<string> &rData);
int getLength(const void *pData);

};
