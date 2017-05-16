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

	explicit CServerAccessLog(CObject *object);
	virtual ~CServerAccessLog();
	int onAccesslog(int nSocket, int nCommand, int nSequence, const void *pData);

protected:

private:

	string paseBody(const void *szBody);
	CObject * mpController;

};
