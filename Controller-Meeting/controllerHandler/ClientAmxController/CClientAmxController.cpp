#include "CClientAmxController.h"

#include "LogHandler.h"

void *threadStartRoutine_CClientAmxController_runSomething(void *argv)
{
	_log("[CClientAmxController] threadStartRoutine_CClientAmxController_runWatcher() step in");

	/*auto uadlh = reinterpret_cast<CClientAmxController*>(argv); 
	uadlh->watcherThreadId = pthread_self();
	uadlh->doLoop = true;
	uadlh->runWatcher();			*/
	
	_log("[CClientAmxController] threadStartRoutine_CClientAmxController_runWatcher() step out");
	return 0;
}

CClientAmxController::CClientAmxController(const std::string &serverIp, int userPort, int validationPort) :
	serverIp(serverIp), userPort(userPort), validationPort(validationPort)
{
}
	
CClientAmxController::~CClientAmxController()
{
}


std::string CClientAmxController::getServerIp()
{
	return serverIp;
}
	
int CClientAmxController::getUserPort()
{
	return userPort;
}

int CClientAmxController::getValidationPort()
{
	return validationPort;
}
