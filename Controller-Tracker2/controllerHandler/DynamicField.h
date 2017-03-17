/*
 * DynamicField.h
 *
 *  Created on: Mar 14, 2017
 *      Author: root
 */

#ifndef CONTROLLER_TRACKER2_CONTROLLERHANDLER_DYNAMICField_H_
#define CONTROLLER_TRACKER2_CONTROLLERHANDLER_DYNAMICField_H_
#include <string>
#include <vector>
#include <algorithm>

class CMysqlHandler;
class DeviceField;

class DynamicField
{
public:
	DynamicField();
	void setMySQLInfo(std::string strHost, std::string strUser, std::string strPassword, std::string strIdeasDBName,
			std::string strFieldDBName);
	bool isValidJSONFormat(std::string data);
	virtual ~DynamicField();
	void insertDynamicData(std::string data);
	void printAllCaches();
private:
	int connectDB(std::string strHost, std::string strDB, std::string strUser, std::string strPassword);
	bool updateCacheDeviceID();
	bool updateCacheDeviceFieldData();
	std::string compareDeviceIDExist(std::string id);
	bool addCacheDeviceFieldData(std::string id,std::vector<std::string> &fieldData);
	std::string getJSONKeyAndID(std::string data, std::vector<std::string> & fieldData);

	CMysqlHandler *ideasDBHandler;
	CMysqlHandler *fieldDBHandler;

	std::vector<DeviceField> *cacheDeviceFieldData;
	std::vector<std::string> * cacheDeviceID;

	std::string strHost;
	std::string strUser;
	std::string strPassword;
	std::string strIdeasDBName;
	std::string strFieldDBName;
	bool isConnectedIdeasDB;
	bool isConnectedFieldDB;

};

#endif /* CONTROLLER_TRACKER2_CONTROLLERHANDLER_DYNAMICField_H_ */
