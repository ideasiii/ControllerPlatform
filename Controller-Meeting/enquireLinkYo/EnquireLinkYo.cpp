#include "EnquireLinkYo.h"

#include "CCmpClient.h"
#include "LogHandler.h"

#define DEFAULT_REQUEST_INTERVAL 10 // seconds
#define DEFAULT_MAX_BALANCE 3

void *threadStartRoutine_EnquireLinkYo_run(void *argv)
{
	auto ely = reinterpret_cast<EnquireLinkYo*>(argv); 
	_log("[EnquireLinkYo] %s threadStartRoutine_EnquireLinkYo_run() step in", ely->strTaskName.c_str());

	ely->loopThreadId = pthread_self();
	ely->isRunning = true;
	ely->run();
	ely->stop();

	_log("[EnquireLinkYo] %s threadStartRoutine_EnquireLinkYo_run() step out", ely->strTaskName.c_str());
	return 0;
}

EnquireLinkYo::EnquireLinkYo(std::string taskName, CCmpClient *client) :
	strTaskName(taskName), client(client), loopThreadId(0), balance(0), loopThreadId(0), 
	isRunning(false), requestInterval(DEFAULT_REQUEST_INTERVAL), 
	maxBalance(DEFAULT_MAX_BALANCE), sockFd(-1)
{
}

EnquireLinkYo::~EnquireLinkYo()
{
	stop();
}

void EnquireLinkYo::setRequestInterval(int value)
{
	if (value <= 0)
	{
		_log("[EnquireLinkYo] setRequestInterval() value %d is invalid", value);
	}
	else
	{
		_log("[EnquireLinkYo] setRequestInterval() value set to %d", value);
		requestInterval = value;
	}
}

void EnquireLinkYo::setMaxBalance(int value)
{
	if (value <= 0)
	{
		_log("[EnquireLinkYo] setMaxBalance() value %d is invalid", value);
	}
	else
	{
		_log("[EnquireLinkYo] setMaxBalance() value set to %d", value);
		maxBalance = value;
	}
}

void EnquireLinkYo::start()
{
	if (isRunning)
	{
		_log("[EnquireLinkYo] %s start() ignored", strTaskName.c_str());
		return;
	}

	_log("[EnquireLinkYo] %s start() creating thread", strTaskName.c_str());
	createThread(threadStartRoutine_EnquireLinkYo_run,
		this, strTaskName.c_str());
}

void EnquireLinkYo::stop()
{
	if (!isRunning)
	{
		_log("[EnquireLinkYo] %s stop() ignored", strTaskName.c_str());
		return;
	}

	isRunning = false;

	if (loopThreadId > 0)
	{
		_log("[EnquireLinkYo] %s stop() cancelling thread", ely->strTaskName.c_str());
		threadCancel(loopThreadId);
		threadJoin(loopThreadId);
		loopThreadId = 0;
	}
	else
	{
		_log("[EnquireLinkYo] %s stop() skip cancelling thread", ely->strTaskName.c_str());
	}
}

void EnquireLinkYo::run()
{
	_DBG("[EnquireLinkYo] %s run() step in", strTaskName.c_str());

	while (true)
	{
		threadSleep(requestInterval);
		
		if (balance >= maxBalance)
		{
			// Enquire Link Failed
			// TODO 怎麼通過 message queue 通知外面 socket 爛了?
			// sockFd 掛了，只能退出迴圈
			_log("[EnquireLinkYo] %s run() reached maxBalance %d, assume broken pipe", strTaskName.c_str(), maxBalance);
			break;
		}

		_log("[EnquireLinkYo] %s run() Sending enquire link request", strTaskName.c_str());

		int nRet = client->request(client->getSocketFd(), enquire_link_request, STATUS_ROK, getSequence(), NULL);

		if (nRet > 0)
		{
			//Enquire Link Success
			//Really?
			balance++;
		}
		else
		{
			//Enquire Link Failed
			// TODO 怎麼通過 message queue 通知外面 socket 爛了?
			_log("[EnquireLinkYo] %s run() Send enquire link failed, result = %d\n", strTaskName.c_str(), nRet);
			// sockFd 掛了，只能退出迴圈
			break;
		}
	}

	log("[EnquireLinkYo] %s run() step out", strTaskName.c_str());
}
