#include "EnquireLinkYo.h"

#include <sys/prctl.h>
#include "event.h"
#include "packet.h"
#include "CCmpClient.h"
#include "LogHandler.h"

#define DEFAULT_REQUEST_INTERVAL 10 // seconds
#define DEFAULT_MAX_BALANCE 3

#define LOG_TAG "[EnquireLinkYo]"

void *threadStartRoutine_EnquireLinkYo_run(void *argv)
{
	pthread_detach(pthread_self());

	auto ely = reinterpret_cast<EnquireLinkYo*>(argv);
	_log(LOG_TAG" %s threadStartRoutine_EnquireLinkYo_run() step in", ely->yoIdentifier.c_str());
	prctl(PR_SET_NAME, (unsigned long)ely->yoIdentifier.c_str());

	ely->loopThreadId = pthread_self();
	ely->run();

	_log(LOG_TAG" %s threadStartRoutine_EnquireLinkYo_run() step out", ely->yoIdentifier.c_str());
	return 0;
}

EnquireLinkYo::EnquireLinkYo(std::string yoIdentifier, CCmpClient *client, int nCommandOnDisconnect, CObject *controller) :
	yoIdentifier(yoIdentifier), client(client), mpController(controller),
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
		_log(LOG_TAG" %s setRequestInterval() value %d is invalid", yoIdentifier.c_str(), value);
	}
	else
	{
		requestInterval = value;
		_log(LOG_TAG" %s setRequestInterval() value set to %d", yoIdentifier.c_str(), value);
	}
}

void EnquireLinkYo::setMaxBalance(int value)
{
	if (value <= 0)
	{
		_log(LOG_TAG" %s setMaxBalance() value %d is invalid", yoIdentifier.c_str(), value);
	}
	else
	{
		_log(LOG_TAG" %s setMaxBalance() value set to %d", yoIdentifier.c_str(), value);
		maxBalance = value;
	}
}

void EnquireLinkYo::start()
{
	if (isRunning)
	{
		_log(LOG_TAG" %s start() ignored", yoIdentifier.c_str());
		return;
	}

	_log(LOG_TAG" %s start() creating thread", yoIdentifier.c_str());
	createThread(threadStartRoutine_EnquireLinkYo_run,
		this, yoIdentifier.c_str());
}

void EnquireLinkYo::stop()
{
	if (!isRunning)
	{
		_log(LOG_TAG" %s stop() ignored", yoIdentifier.c_str());
		return;
	}

	isRunning = false;

	if (loopThreadId > 0)
	{
		_log(LOG_TAG" %s stop() cancelling thread", yoIdentifier.c_str());
		threadCancel(loopThreadId);
		loopThreadId = 0;
	}
	else
	{
		_log(LOG_TAG" %s stop() skip cancelling thread", yoIdentifier.c_str());
	}

	_log(LOG_TAG" %s stop() stopped EnquireLinkYo", yoIdentifier.c_str());
}

void EnquireLinkYo::zeroBalance()
{
	balance = 0;
}

void EnquireLinkYo::run()
{
	//_log(LOG_TAG" %s run() step in", yoIdentifier.c_str());

	if (isRunning)
	{
		_log(LOG_TAG" %s run() isRunning == true, quit before looping", yoIdentifier.c_str());
		return;
	}

	isRunning = true;
	balance = 0;

	if (!client->isValidSocketFD())
	{
		_log(LOG_TAG" %s run() sock fd is not valid, quit before looping", yoIdentifier.c_str());
		isRunning = false;
		return;
	}

	while (isRunning)
	{
		sleep(requestInterval);

		if (!isRunning)
		{
			break;
		}

		// isValidSocketFD() 僅檢查內部的 sock fd 是不是 -1
		// 不會檢查 sock fd 是否可以送封包
		// 如果 CCmpClient 在連到 server 之後才啟動 EnquireLinkYo 的話
		// 這個檢查並沒有用, 因為 sock fd 絕對不會是 -1
		// if (!client->isValidSocketFD())
		// {
		// }

		if (balance >= maxBalance)
		{
			_log(LOG_TAG" %s run() assume broken pipe (reached maxBalance %d)", yoIdentifier.c_str(), maxBalance);
			informEnquireLinkFailure();

			// socket 壞了，只能退出迴圈
			break;
		}

		//_log(LOG_TAG" %s run() Sending enquire link request", yoIdentifier.c_str());

		int nRet = client->request(client->getSocketfd(), enquire_link_request, STATUS_ROK, getSequence(), NULL);
		if (!isRunning)
		{
			break;
		}

		if (nRet > 0)
		{
			balance++;
		}
		else
		{
			_log(LOG_TAG" %s run() Send enquire link failed, result = %d", yoIdentifier.c_str(), nRet);
			informEnquireLinkFailure();

			// socket 壞了，只能退出迴圈
			break;
		}
	}

	_log(LOG_TAG" %s run() step out", yoIdentifier.c_str());
}

void EnquireLinkYo::informEnquireLinkFailure()
{
	//_log(LOG_TAG" %s informEnquireLinkFailure() step in", yoIdentifier.c_str());

	auto dyingMessage = yoIdentifier + " disconnect (1)....";
	mpController->sendMessage(commandOnDisconnect, 0, dyingMessage.length(), dyingMessage.c_str());

	sleep(1);
	if (!isRunning)
	{
		return;
	}

	// First event sometimes dropped by external receiver
	// So send it twice!?
	dyingMessage = yoIdentifier + " disconnect (2)....";
	mpController->sendMessage(commandOnDisconnect, 0, dyingMessage.length(), dyingMessage.c_str());
}

// this class does not receive events from message queue
void EnquireLinkYo::onReceiveMessage(int lFilter, int nCommand, unsigned long int nId, int nDataLen,
	const void* pData)
{
	_log(LOG_TAG" %s onReceiveMessage() what the hell?", yoIdentifier.c_str());
}

// this class does not receive events from message queue
void EnquireLinkYo::onHandleMessage(Message &message)
{
	_log(LOG_TAG" %s onHandleMessage() what the hell?", yoIdentifier.c_str());
}

std::string EnquireLinkYo::taskName()
{
	return yoIdentifier;
}
