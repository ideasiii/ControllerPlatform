/*
 * CSqlite.cpp
 *
 *  Created on: Apr 19, 2016
 *      Author: joe
 */

#include "CSqlite.h"
#include "CSqliteHandler.h"
#include <algorithm>
#include "CThreadHandler.h"
#include "common.h"
#include "event.h"
#include "cJSON.h"
#include "LogHandler.h"
using namespace std;

static CSqlite *mInstance = 0;
static vector<DeviceField> cacheDeviceFieldData = vector<DeviceField>();
static vector<string> cacheDeviceIDData = vector<string>();

class DeviceField
{
private:
	std::string device_id;
	std::string field_name;

public:

	DeviceField(std::string device_id, std::string field_name) :
			device_id(device_id), field_name(field_name)
	{
	}
	string getDeviceID()
	{
		return device_id;
	}
	string getFieldName()
	{
		return field_name;
	}

	bool operator ==(const DeviceField& rhs) const
	{
		if (this->device_id.compare(rhs.device_id) == 0 && this->field_name.compare(rhs.field_name) == 0)
		{
			return true;
		}

		return false;
	}

};

struct ThreadArgv
{
	CSqlite *mCsqlite;
	string jsonString;
};

CSqlite::CSqlite() :
		threadHandler(new CThreadHandler()), mCSqliteHandler(CSqliteHandler::getInstance())
{

}

CSqlite::~CSqlite()
{
}

CSqlite *CSqlite::getInstance()
{
	if (!mInstance)
	{
		mInstance = new CSqlite();
	}

	return mInstance;
}

/***************** Thread Function *********************/
void *threadSqliteHandler(void *argv)
{

	ThreadArgv* ss = reinterpret_cast<ThreadArgv*>(argv);

	cJSON *cJsonInputString = cJSON_Parse(ss->jsonString.c_str());

	string deviceID = "";
	vector<string> fieldData = vector<string>();

	if (cJsonInputString)
	{
		cJSON *cJsonData = cJsonInputString->child;
		cJSON *next;


		while (cJsonData)
		{
			next = cJsonData->next;

			if (strcmp(cJsonData->string, "ID") == 0)
			{
				deviceID = cJsonData->valuestring;
			}
			else
			{
				fieldData.push_back(cJsonData->string);
			}
			cJsonData = next;
		}

		delete cJsonData;
		delete next;
		delete cJsonInputString;
		cJsonData = NULL;
		next = NULL;
	}
	else
	{
		_log("[CSqlite] Invaild Json format!!  data: %s", ss->jsonString.c_str());
	}
	cJsonInputString = NULL;

	ss->mCsqlite->updateDeviceFieldTable(deviceID, fieldData);

	return NULL;
}

/***************** Thread Function *********************/
void *threadCSqliteMessageReceive(void *argv)
{
	CSqlite* ss = reinterpret_cast<CSqlite*>(argv);
	ss->initMessage(MSG_ID);
	ss->runMessageReceive();
	return NULL;
}

void CSqlite::runMessageReceive()
{
	//_DBG("[CSqlite] ********in run Message Receive ********")
	run( EVENT_FILTER_CSQLITE);
	threadHandler->threadExit();
	threadHandler->threadJoin(threadHandler->getThreadID());
}

void CSqlite::createThread(string jsonString)
{
	ThreadArgv *argv = new ThreadArgv();
	argv->mCsqlite = this;

	argv->jsonString = jsonString;

	threadHandler->createThread(threadSqliteHandler, (void *) argv);

}
void CSqlite::createMessageReceiver()
{
	threadHandler->createThread(threadCSqliteMessageReceive, this);
}

bool CSqlite::updateCacheDeviceFieldData()
{
	cacheDeviceFieldData.clear();
	vector<vector<string> > newSqlData = vector<vector<string> >();

	int dataCount = mCSqliteHandler->fieldSqlExec("SELECT id, field FROM device_field;", newSqlData, 2);

	if (dataCount == 0)
	{
		return false;
	}

	for (size_t i = 0; i < newSqlData.size(); i++)
	{
		cacheDeviceFieldData.push_back(DeviceField(newSqlData.at(i).at(0), newSqlData.at(i).at(1)));
	}

	return true;
}

void CSqlite::updateCacheDeviceIDData()
{
	cacheDeviceIDData.clear();
	list<string> *newSqlData = new list<string>();
	int dataCount = mCSqliteHandler->ideasSqlExec("SELECT app_id FROM app;", *newSqlData, 0);

	//_DBG("data count: %d", dataCount)

	for (list<string>::iterator it = newSqlData->begin(); it != newSqlData->end(); ++it)
	{
		cacheDeviceIDData.push_back(string(*it));
	}
	/*
	 for (size_t i = 0; i < cacheDeviceIDData.size(); i++)
	 {
	 _DBG("update new device id: %s", cacheDeviceIDData.at(i).c_str())
	 }
	 */
	delete newSqlData;

}

string CSqlite::findCacheDeviceIDExist(string id)
{
	for (size_t i = 0; i < cacheDeviceIDData.size(); i++)
	{
		if (id.find(cacheDeviceIDData.at(i)) != string::npos)
		{
			return cacheDeviceIDData.at(i);
		}
	}
	return "";
}
bool CSqlite::findCacheDeviceFieldDataExist(string deviceID, string fieldName)
{
	DeviceField item = DeviceField(deviceID, fieldName);

	if (find(cacheDeviceFieldData.begin(), cacheDeviceFieldData.end(), item) != cacheDeviceFieldData.end())
	{
		return true;
	}
	return false;

}

void CSqlite::updateDeviceFieldTable(string id, vector<string> &mData)
{
	//_DBG("***** IN updateDeviceFieldTable *********")

	//_DBG("id = %s", id.c_str());
	//for (size_t i = 0; i < mData.size(); i++)
	//	_DBG("flied data = %s", mData.at(i).c_str());

	threadHandler->threadLock();

	//here add rawdata ID to compare cacheDeviceID cause rawdata ID is composed by mac+deviceID+mail

	string deviceID = findCacheDeviceIDExist(id);
	if (deviceID.empty())
	{
		updateCacheDeviceIDData();
		deviceID = findCacheDeviceIDExist(id);
	}

	if (deviceID.empty())
	{
		//error nonknown this id
		//recorded it
		_log("[CSqlite] UNKNOWN this Device ID %s", id.c_str());

		sendMessage(EVENT_FILTER_CSQLITE, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);
		threadHandler->threadUnlock();
		threadHandler->threadSleep(1);
		threadHandler->threadExit();
		return;
	}
	vector<DeviceField> *needToAdd = new vector<DeviceField>();

	for (size_t i = 0; i < mData.size(); i++)
	{
		if (findCacheDeviceFieldDataExist(deviceID, mData.at(i)) == false)
		{
			needToAdd->push_back(DeviceField(deviceID, mData.at(i)));
		}
	}

	for (size_t i = 0; i < needToAdd->size(); i++)
	{

		string sqlexec = "INSERT INTO device_field(id,field) VALUES('" + needToAdd->at(i).getDeviceID() + "','"
				+ needToAdd->at(i).getFieldName() + "');";

		mCSqliteHandler->fieldSqlExec(sqlexec.c_str());

	}

	if (needToAdd->size() != 0)
	{
		if (updateCacheDeviceFieldData() == false)
		{
			//error
			_log("[CSqlite] some ERROR from sqlite which in updating Device Field Chache");
		}
	}

	needToAdd->clear();
	delete needToAdd;
	needToAdd = NULL;

	sendMessage(EVENT_FILTER_CSQLITE, EVENT_COMMAND_THREAD_EXIT, threadHandler->getThreadID(), 0, NULL);
	threadHandler->threadUnlock();
	threadHandler->threadSleep(1);
	threadHandler->threadExit();
}

void CSqlite::onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData)
{

	switch (nCommand)
	{
	case EVENT_COMMAND_THREAD_EXIT:
		threadHandler->threadJoin(nId);
		_DBG("[CSqlite] Thread Join:%d", (int )nId);
		break;
	case EVENT_COMMAND_DATA:
	{
		void* ss = const_cast<void*>(pData);
		char* sss = reinterpret_cast<char*>(ss);

		cJSON *cJsonInputString = cJSON_Parse(sss);
		cJSON *cJsonData = cJsonInputString->child;
		cJSON *next;

		string deviceID = "";
		vector<string> fieldData = vector<string>();
		while (cJsonData)
		{
			next = cJsonData->next;

			if (strcmp(cJsonData->string, "ID") == 0)
			{
				deviceID = cJsonData->valuestring;
			}
			else
			{
				fieldData.push_back(cJsonData->string);
			}
			cJsonData = next;
		}

		//_DBG("id = %s", deviceID.c_str());
		//for (size_t i = 0; i < fieldData.size(); i++)
		//{
		//	_DBG("flied data = %s", fieldData.at(i).c_str());
		//}
		this->updateDeviceFieldTable(deviceID, fieldData);

		delete cJsonInputString;
		delete cJsonData;
		delete next;
		cJsonInputString = NULL;
		cJsonData = NULL;
		next = NULL;

		break;
	}
	default:
		_DBG("[CSqlite]********* Unknown message command*********");
		break;
	}

}

