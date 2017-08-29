/*
 * DynamicField.cpp
 *
 *  Created on: Mar 14, 2017
 *      Author: root
 */

#include "DynamicField.h"
#include "CMysqlHandler.h"
#include "JSONObject.h"
#include "common.h"
#include "LogHandler.h"
#include "cJSON.h"

using namespace std;

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
		if(this->device_id.compare(rhs.device_id) == 0 && this->field_name.compare(rhs.field_name) == 0)
		{
			return true;
		}

		return false;
	}

};

DynamicField::DynamicField() :
		ideasDBHandler(new CMysqlHandler), fieldDBHandler(new CMysqlHandler), cacheDeviceFieldData(
				new vector<DeviceField>), cacheDeviceID(new vector<string>), isConnectedIdeasDB(false), isConnectedFieldDB(
				false)
{

}

void DynamicField::setMySQLInfo(string strHost, string strUser, string strPassword, string strIdeasDBName,
		string strFieldDBName)
{
	this->strHost = strHost;
	this->strIdeasDBName = strIdeasDBName;
	this->strUser = strUser;
	this->strPassword = strPassword;
	this->strFieldDBName = strFieldDBName;
}

bool DynamicField::isValidJSONFormat(string data)
{
	bool isValid = false;
	JSONObject *jobjRoot = new JSONObject(data);
	if(jobjRoot->isValid())
	{
		_log("[DynamicField] is Valid JSON Format %s", data.c_str());
		isValid = true;

	}
	else
	{
		_log("[DynamicField] is NOT Valid JSON Format %s", data.c_str());

	}

	jobjRoot->release();
	delete jobjRoot;

	jobjRoot = 0;

	return isValid;
}

bool DynamicField::updateCacheDeviceID()
{

	if(isConnectedIdeasDB == false)
	{
		int status = connectDB(strHost, strIdeasDBName, strUser, strPassword);
		if(status > 0)
		{
			isConnectedIdeasDB = true;
			return updateCacheDeviceID();
		}
		else
		{
			_log("[DynamicField] %s ", ideasDBHandler->getLastError().c_str());
			_log("[DynamicField] Cannot Connect ideasDB!");
			return false;
		}
	}
	else
	{

		string strSQL = "select app_id from app";
		list<map<string, string> > listRest;
		if(ideasDBHandler->query(strSQL, listRest) == FALSE)
		{
			_log("[DynamicField] Cannot Query ideasDB query:%s", strSQL.c_str());
			ideasDBHandler->close();
			isConnectedIdeasDB = false;
			return false;
		}
		else
		{
			ideasDBHandler->close();
			isConnectedIdeasDB = false;

			if(!listRest.empty())
			{
				list<map<string, string>>::iterator iterator;
				cacheDeviceID->clear();

				for(iterator = listRest.begin(); iterator != listRest.end(); ++iterator)
				{

					cacheDeviceID->push_back((*iterator)["app_id"]);
				}

				//For debug START
				/*
				 int nCount = 0;
				 for (list<map<string, string> >::iterator i = listRest.begin(); i != listRest.end(); ++i, ++nCount)
				 {
				 map<string, string> mapItem = *i;
				 for (map<string, string>::iterator j = mapItem.begin(); j != mapItem.end(); ++j)
				 {
				 _log("%s : %s\n", (*j).first.c_str(), (*j).second.c_str());
				 }
				 }
				 _log("=============================%d================================\n", nCount);
				 */
				//For debug END
			}
			else
			{
				_log("[DynamicField] Cannot Query ideasDB query:%s", strSQL.c_str());
				return false;
			}

		}

	}
	return true;
}

void DynamicField::printAllCaches()
{
	_log("[DynamicField] ***********************print cacheDeviceID *******\n");
	for(size_t i = 0; i < this->cacheDeviceID->size(); i++)
	{
		_log("[DynamicField] ***********************id: %s\n", cacheDeviceID->at(i).c_str());
	}
	_log("[DynamicField] ***********************print CacheDeviceFieldData *******\n");
	for(size_t i = 0; i < this->cacheDeviceFieldData->size(); i++)
	{
		_log("[DynamicField] ***********************id: %s field: %s",
				cacheDeviceFieldData->at(i).getDeviceID().c_str(), cacheDeviceFieldData->at(i).getFieldName().c_str());
	}

}

bool DynamicField::addCacheDeviceFieldData(string id, vector<string> &fieldData)
{
	bool isAddData = false;
	for(size_t i = 0; i < fieldData.size(); i++)
	{
		DeviceField tmp = DeviceField(id, fieldData.at(i));
		if(!(find(cacheDeviceFieldData->begin(), cacheDeviceFieldData->end(), tmp) != cacheDeviceFieldData->end()))
		{
			cacheDeviceFieldData->push_back(tmp);
			isAddData = true;
		}

	}
	return isAddData;

}

void DynamicField::insertDynamicData(string data)
{

	vector<string> fieldData;

	string id = getJSONKeyAndID(data, fieldData);
	if(!id.empty())
	{
		string deviceID = compareDeviceIDExist(id);
		if(!deviceID.empty())
		{
			if(addCacheDeviceFieldData(deviceID, fieldData) == true)
			{
				updateCacheDeviceFieldData();
			}
			else
			{
				//do nothing
			}
		}
		else
		{
			//double check, update cache device ID
			if(updateCacheDeviceID() > 0)
			{
				deviceID = compareDeviceIDExist(id);

				if(!deviceID.empty())
				{
					if(addCacheDeviceFieldData(deviceID, fieldData) == true)
					{
						updateCacheDeviceFieldData();
					}
					else
					{
						//do nothing
					}
				}
				else
				{
					_log("[DynamicField] Unknown this ID %s", id.c_str());

				}
			}
			else
			{

			}
		}
	}
	else
	{
		_log("[DynamicField] Can not found ID field");
	}

	fieldData.clear();

}

string DynamicField::compareDeviceIDExist(string id)
{
	string deviceID;

	for(size_t i = 0; i < cacheDeviceID->size(); i++)
	{
		if(id.find(cacheDeviceID->at(i)) != string::npos)
		{
			//found it!
			deviceID = cacheDeviceID->at(i);

		}
	}

	return deviceID;

}

string DynamicField::getJSONKeyAndID(string data, vector<string> & fieldData)
{

	string deviceID;
	JSONObject *jobjRoot = new JSONObject(data);

	cJSON *cJsonInputString = jobjRoot->getcJSON();

	if(cJsonInputString)
	{
		cJSON *cJsonData = cJsonInputString->child;
		cJSON *next;
		while(cJsonData)
		{
			next = cJsonData->next;

			if(strcmp(cJsonData->string, "ID") == 0)
			{
				deviceID = (cJsonData->valuestring);
				fieldData.push_back(cJsonData->string);
			}
			else
			{
				fieldData.push_back(cJsonData->string);
			}
			cJsonData = next;
		}

	}

	jobjRoot->release();
	delete jobjRoot;
	jobjRoot = 0;

	return deviceID;

}

bool DynamicField::updateCacheDeviceFieldData()
{
	if(isConnectedFieldDB == false)
	{
		int status = connectDB(strHost, strFieldDBName, strUser, strPassword);
		if(status > 0)
		{
			isConnectedFieldDB = true;
			return updateCacheDeviceFieldData();
		}
		else
		{

			_log("[DynamicField] Cannot Connect FieldDB!");
			return false;
		}
	}
	else
	{
		for(size_t i = 0; i < cacheDeviceFieldData->size(); i++)
		{

			string strSQL = "insert into device_field (id,field) values('" + cacheDeviceFieldData->at(i).getDeviceID()
					+ "','" + cacheDeviceFieldData->at(i).getFieldName() + "')";
			fieldDBHandler->sqlExec(strSQL);
		}

		fieldDBHandler->close();
		isConnectedFieldDB = false;
	}
	return true;

}

int DynamicField::connectDB(string strHost, string strDB, string strUser, string strPassword)
{
	_log("[DynamicField] now connect to:%s DB:%s User:%s Pwd:%s", strHost.c_str(), strDB.c_str(), strUser.c_str(),
			strPassword.c_str());

	if(!strDB.compare(this->strIdeasDBName))
	{
		return ideasDBHandler->connect(strHost, strDB, strUser, strPassword, "10");
	}
	else if(!strDB.compare(this->strFieldDBName))
	{
		return fieldDBHandler->connect(strHost, strDB, strUser, strPassword, "10");
	}
	else
	{
		return -1;
	}

}

DynamicField::~DynamicField()
{

	ideasDBHandler->close();
	delete ideasDBHandler;
	ideasDBHandler = 0;

	fieldDBHandler->close();
	delete fieldDBHandler;
	fieldDBHandler = 0;

	cacheDeviceFieldData->clear();
	delete cacheDeviceFieldData;
	cacheDeviceFieldData = 0;

	cacheDeviceID->clear();
	delete cacheDeviceID;
	cacheDeviceID = 0;

}

