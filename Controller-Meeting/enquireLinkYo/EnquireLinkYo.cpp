#include "EnquireLinkYo.h"

#include "event.h"
#include "packet.h"
#include "CCmpClient.h"
#include "LogHandler.h"

#define DEFAULT_REQUEST_INTERVAL 10 // seconds
#define DEFAULT_MAX_BALANCE 1

#define LOG_TAG "[EnquireLinkYo]"

void *threadStartRoutine_EnquireLinkYo_run(void *argv)
{
	auto ely = reinterpret_cast<EnquireLinkYo*>(argv); 
	_log(LOG_TAG" %s threadStartRoutine_EnquireLinkYo_run() step in", ely->whoUsedMe.c_str());

	ely->loopThreadId = pthread_self();
	ely->run();

	_log(LOG_TAG" %s threadStartRoutine_EnquireLinkYo_run() step out", ely->whoUsedMe.c_str());
	return 0;
}

EnquireLinkYo::EnquireLinkYo(std::string whoUsedMe, CCmpClient *client, int nCommandOnDisconnect, CObject *controller) :
	whoUsedMe(whoUsedMe), client(client), mpController(controller),
	commandOnDisconnect(nCommandOnDisconnect), loopThreadId(0), isRunning(false),
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
		_log(LOG_TAG" %s setRequestInterval() value %d is invalid", whoUsedMe.c_str(), value);
	}
	else
	{
		_log(LOG_TAG" %s setRequestInterval() value set to %d", whoUsedMe.c_str(), value);
		requestInterval = value;
	}
}

void EnquireLinkYo::setMaxBalance(int value)
{
	if (value <= 0)
	{
		_log(LOG_TAG" %s setMaxBalance() value %d is invalid", whoUsedMe.c_str(), value);
	}
	else
	{
		_log(LOG_TAG" %s setMaxBalance() value set to %d", whoUsedMe.c_str(), value);
		maxBalance = value;
	}
}

void EnquireLinkYo::start()
{
	if (isRunning)
	{
		_log(LOG_TAG" %s start() ignored", whoUsedMe.c_str());
		return;
	}

	_log(LOG_TAG" %s start() creating thread", whoUsedMe.c_str());
	createThread(threadStartRoutine_EnquireLinkYo_run,
		this, whoUsedMe.c_str());
}

void EnquireLinkYo::stop()
{
	if (!isRunning)
	{
		_log(LOG_TAG" %s stop() ignored", whoUsedMe.c_str());
		return;
	}

	isRunning = false;

	if (loopThreadId > 0)
	{
		_log(LOG_TAG" %s stop() cancelling thread", whoUsedMe.c_str());
		threadCancel(loopThreadId);
		pthread_detach(loopThreadId);
		//threadJoin(loopThreadId);

		loopThreadId = 0;
	}
	else
	{
		_log(LOG_TAG" %s stop() skip cancelling thread", whoUsedMe.c_str());
	}

	_log(LOG_TAG" %s stop() stopped EnquireLinkYo", whoUsedMe.c_str());
}

void EnquireLinkYo::zeroBalance()
{
	balance = 0;
}

void EnquireLinkYo::run()
{
	_DBG(LOG_TAG" %s run() step in", whoUsedMe.c_str());

	if (isRunning)
	{
		_log(LOG_TAG" %s run() isRunning == true, quit before looping", whoUsedMe.c_str());
		return;
	}

	isRunning = true;
	balance = 0;
	
	if (!client->isValidSocketFD())
	{
		_log(LOG_TAG" %s run() sock fd is not valid, quit before looping", whoUsedMe.c_str());
		isRunning = false;
		return;
	}
	
	//int crashCounter = 0;
	
	while (isRunning)
	{
		sleep(requestInterval);
		
		if (!isRunning)
		{
			break;
		}

		/*
		// isValidSocketFD() 只是檢查內部的 sock fd 是不是 -1
		// 不是檢查 sock fd 是否可以送封包
		// 所以如果 CCmpClient 在確定連到 server 之後才啟動這個 function 的話
		// 這個檢查並沒有用
		if (!client->isValidSocketFD())
		{
			informEnquireLinkFailure();

			_log(LOG_TAG" %s run() client socket not valid, return", strTaskName.c_str());
			// socket 壞了，只能退出迴圈
			break;
		}*/

		/*if (crashCounter++ > 5)
		{
			// test message sending
			_log(LOG_TAG" %s run() assume broken pipe (crashCounter > 5)", whoUsedMe.c_str());
			informEnquireLinkFailure();
			break;
		}*/

		if (balance >= maxBalance)
		{
			// Enquire Link Failed
			_log(LOG_TAG" %s run() assume broken pipe (reached maxBalance %d)", whoUsedMe.c_str(), maxBalance);
			informEnquireLinkFailure();
			
			// socket 壞了，只能退出迴圈
			break;
		}

		_log(LOG_TAG" %s run() Sending enquire link request", whoUsedMe.c_str());

		int nRet = client->request(client->getSocketfd(), enquire_link_request, STATUS_ROK, getSequence(), NULL);
		if (!isRunning)
		{
			break;
		}

		if (nRet > 0)
		{
			// Enquire Link Success
			// Really?
			balance++;
		}
		else
		{
			_log(LOG_TAG" %s run() Send enquire link failed, result = %d\n", whoUsedMe.c_str(), nRet);
			informEnquireLinkFailure();
			
			// socket 壞了，只能退出迴圈
			break;
		}
	}

	_log(LOG_TAG" %s run() step out", whoUsedMe.c_str());
}

void EnquireLinkYo::informEnquireLinkFailure()
{
	//_log(LOG_TAG" %s informEnquireLinkFailure() step in", whoUsedMe.c_str());

	auto dyingMessage = whoUsedMe + " disconnect (1)....";
	mpController->sendMessage(commandOnDisconnect, 0, dyingMessage.length(), dyingMessage.c_str());

	sleep(1);
	if (!isRunning)
	{
		return;
	}

	// First event sometimes dropped by external receiver
	// So send message twice!?
	dyingMessage = whoUsedMe + " disconnect (2)....";
	mpController->sendMessage(commandOnDisconnect, 0, dyingMessage.length(), dyingMessage.c_str());
}

// this class does not receive events from message queue
void EnquireLinkYo::onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
	const void* pData)
{
	_log(LOG_TAG" %s onReceiveMessage() what the hell?", whoUsedMe.c_str());
}

// this class does not receive events from message queue
void EnquireLinkYo::onHandleMessage(Message &message)
{
	_log(LOG_TAG" %s onHandleMessage() what the hell?", whoUsedMe.c_str());
}

std::string EnquireLinkYo::taskName()
{
	return whoUsedMe;
}
