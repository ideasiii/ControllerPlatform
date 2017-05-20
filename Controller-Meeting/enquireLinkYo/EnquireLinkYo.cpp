#include "EnquireLinkYo.h"

#include "../controllerHandler/iCommand.h"
#include "event.h"
#include "packet.h"
#include "CCmpClient.h"
#include "LogHandler.h"

#define DEFAULT_REQUEST_INTERVAL 10 // seconds
#define DEFAULT_MAX_BALANCE 3

void *threadStartRoutine_EnquireLinkYo_run(void *argv)
{
	auto ely = reinterpret_cast<EnquireLinkYo*>(argv); 
	_log("[EnquireLinkYo] %s threadStartRoutine_EnquireLinkYo_run() step in", ely->strTaskName.c_str());

	ely->loopThreadId = pthread_self();
	ely->run();
	ely->stop();

	_log("[EnquireLinkYo] %s threadStartRoutine_EnquireLinkYo_run() step out", ely->strTaskName.c_str());
	return 0;
}

EnquireLinkYo::EnquireLinkYo(std::string taskName, CCmpClient *client, int messageWhat, CObject *controller) :
	strTaskName(taskName), client(client), mpController(controller),
	messageWhat(messageWhat), loopThreadId(0), isRunning(false),
	requestInterval(DEFAULT_REQUEST_INTERVAL), maxBalance(DEFAULT_MAX_BALANCE), balance(0)
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
		_log("[EnquireLinkYo] %s stop() cancelling thread", strTaskName.c_str());
		threadCancel(loopThreadId);
		threadJoin(loopThreadId);
		loopThreadId = 0;
	}
	else
	{
		_log("[EnquireLinkYo] %s stop() skip cancelling thread", strTaskName.c_str());
	}
}

void EnquireLinkYo::zeroBalance()
{
	balance = 0;
}

void EnquireLinkYo::run()
{
	_DBG("[EnquireLinkYo] %s run() step in", strTaskName.c_str());

	if (isRunning)
	{
		_log("[EnquireLinkYo] %s run() isRunning == true, quit before looping", strTaskName.c_str());
		return;
	}

	isRunning = true;
	balance = 0;
	
	if (!client->isValidSocketFD())
	{
		_log("[EnquireLinkYo] %s run() sock fd is not valid, quit before looping", strTaskName.c_str());
		isRunning = false;
		return;
	}

	while (true)
	{
		sleep(requestInterval);
		
		/*
		// isValidSocketFD() 只是檢查內部的 sock fd 是不是 -1
		// 不是檢查 sock fd 是否可以送封包
		// 所以如果 CCmpClient 在確定連到 server 之後才啟動這個 function 的話
		// 這個檢查並沒有用
		if (!client->isValidSocketFD())
		{
			informEnquireLinkFailure();

			_log("[EnquireLinkYo] %s run() client socket not valid, return", strTaskName.c_str());
			// socket 壞了，只能退出迴圈
			break;
		}*/

		if (balance >= maxBalance)
		{
			// Enquire Link Failed
			informEnquireLinkFailure();

			_log("[EnquireLinkYo] %s run() reached maxBalance %d, assume broken pipe", strTaskName.c_str(), maxBalance);
			// socket 壞了，只能退出迴圈
			break;
		}

		_log("[EnquireLinkYo] %s run() Sending enquire link request", strTaskName.c_str());

		int nRet = client->request(client->getSocketfd(), enquire_link_request, STATUS_ROK, getSequence(), NULL);

		if (nRet > 0)
		{
			//Enquire Link Success
			//Really?
			balance++;
		}
		else
		{
			//Enquire Link Failed
			informEnquireLinkFailure();
			_log("[EnquireLinkYo] %s run() Send enquire link failed, result = %d\n", strTaskName.c_str(), nRet);
			
			// socket 壞了，只能退出迴圈
			break;
		}
	}

	_log("[EnquireLinkYo] %s run() step out", strTaskName.c_str());
}

void EnquireLinkYo::informEnquireLinkFailure()
{
	//Enquire Link Failed
			Message message;
			message.what = MESSAGE_EVENT_CLIENT_MEETING_AGENT;
			message.arg[0] = -1;
			mpController->sendMessage(message);
}

// this class does not receive events from message queue
void EnquireLinkYo::onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
	const void* pData)
{
	_log("[EnquireLinkYo] onReceiveMessage() what the hell?");
}

// this class does not receive events from message queue
void EnquireLinkYo::onHandleMessage(Message &message)
{
	_log("[EnquireLinkYo] onHandleMessage() what the hell?");
}

std::string EnquireLinkYo::taskName()
{
	return "EnquireLinkYo";
}
