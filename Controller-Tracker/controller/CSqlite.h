/*
 * CSqlite.h
 *
 *  Created on: Apr 19, 2016
 *      Author: root
 */

#ifndef CONTROLLER_CSQLITE_H_
#define CONTROLLER_CSQLITE_H_
#include "CSqliteHandler.h"
#include "CObject.h"
#include <string>
#include <vector>
class CSqliteHandler;
class CThreadHandler;



class DeviceField;


class CSqlite :public CObject
{
	enum EVENT_INTERNAL
	{
		EVENT_COMMAND_THREAD_EXIT = 0,EVENT_COMMAND_DATA
	};
public:
	static CSqlite *getInstance();
	CSqlite();
	virtual ~CSqlite();
	void createMessageReceiver();

	void updateDeviceFieldTable(std::string id,std::vector<std::string> &mData);
	bool updateCacheDeviceFieldData();
	void updateCacheDeviceIDData();
	void runMessageReceive();
	void createThread(std::string jsonString);

protected:
		void onReceiveMessage(int nEvent, int nCommand, unsigned long int nId, int nDataLen, const void* pData);

private:
	CThreadHandler *threadHandler;
	CSqliteHandler *mCSqliteHandler;
	bool findCacheDeviceFieldDataExist(std::string deviceID,std::string fieldName);
	std::string findCacheDeviceIDExist(std::string id);
};


#endif /* CONTROLLER_CSQLITE_H_ */
