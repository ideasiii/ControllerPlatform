/*
 * CMongoDBHandler.cpp
 *
 *  Created on: 2014年12月15日
 *      Author: jugo
 */

#include <iostream>
#include <set>
#include <cstdlib>
#include <string>
#include <map>
#include <memory>
#include "mongo/client/dbclient.h"
#include "common.h"
#include "CMongoDBHandler.h"
#include "utility.h"
#include "packet.h"
#include "LogHandler.h"

#ifndef verify
#define verify(x) MONGO_verify(x)
#endif

using namespace std;
using namespace mongo;

CMongoDBHandler* CMongoDBHandler::mInstance = 0;
static DBClientConnection *DBconn = 0;

CMongoDBHandler::CMongoDBHandler()
{
	client::GlobalInstance instance;
	if (!instance.initialized())
	{
		std::cout << "failed to initialize the client driver: " << instance.status() << std::endl;
		_log("[CMongoDBHandler] Initialized Mongodb Fail");
		return;
	}
	_log("[CMongoDBHandler] Initialized Mongodb Client Driver Success");

}
CMongoDBHandler::~CMongoDBHandler()
{
	close();
}

CMongoDBHandler* CMongoDBHandler::getInstance()
{
	if (!mInstance)
	{
		mInstance = new CMongoDBHandler();
	}

	return mInstance;
}

int CMongoDBHandler::connectDB()
{
	close();
	DBconn = new DBClientConnection();
	string uri = "mongodb://127.0.0.1:27017";
	string errmsg;

	ConnectionString cs = ConnectionString::parse(uri, errmsg);

	if (!cs.isValid())
	{
		std::cout << "Error parsing connection string " << uri << ": " << errmsg << std::endl;
		return FALSE;
	}

	boost::scoped_ptr<DBClientBase> conn(cs.connect(errmsg));

	if (!conn)
	{
		cout << "couldn't connect : " << errmsg << endl;
		return FALSE;
	}

	_log("[CMongoDBHandler] Connect DB Success");

	return TRUE;
}
int CMongoDBHandler::connectDB(string strIP, string strPort)
{
	close();
	DBconn = new DBClientConnection();
	string errmsg;
	int nRet = FALSE;

	if (!DBconn->connect(strIP + ":" + strPort, errmsg))
	{
		_log("[CMongoDBHandler] DB Connect Fail! , Error: %s", errmsg.c_str());
	}
	else
	{
		nRet = TRUE;
		_log("[CMongoDBHandler] DB Connected");
	}

	return nRet;
}

int CMongoDBHandler::connectDB(std::string strIP, std::string strPort, std::string strDBName, std::string strUser,
		std::string strPasswd)
{
	close();
	DBconn = new DBClientConnection();
	string strErrMsg;
	int nRet = FALSE;

	if (!DBconn->connect(strIP + ":" + strPort, strErrMsg))
	{
		_log("[CMongoDBHandler] Couldn't connect:%s", strErrMsg.c_str());
	}
	else
	{
		bool ok = DBconn->auth(strDBName, strUser, strPasswd, strErrMsg);
		if (!ok)
		{
			_log("[CMongoDBHandler] %s", strErrMsg.c_str());
		}
		else
		{
			nRet = TRUE;
			_log("[CMongoDBHandler] DB Connected , DB:%s User:%s Password:%s ErrorMsg:%s", strDBName.c_str(),
					strUser.c_str(), strPasswd.c_str(), strErrMsg.c_str());
		}
	}

	return nRet;
}

void CMongoDBHandler::close()
{
	if (0 != DBconn)
	{
		delete DBconn;
		DBconn = 0;
	}
}

void CMongoDBHandler::insert(std::string strDB, std::string strCollection, std::map<std::string, std::string> &mapData)
{
	if (!isValid())
		return;
	string strCon = strDB + "." + strCollection;
	map<string, string>::iterator it;
	BSONObjBuilder b;

	for (it = mapData.begin(); it != mapData.end(); ++it)
	{
		b.append(it->first, it->second);
	}

	b.append("record_state", 0);
	b.append("create_date", currentDateTime());
	BSONObj p = b.obj();
	DBconn->insert(strCon, p);
}

void CMongoDBHandler::insert(std::string strDB, std::string strCollection, std::string strColumn, std::string strValue)
{
	if (!isValid())
		return;

	string strCon = strDB + "." + strCollection;
	BSONObjBuilder b;
	b.append(strColumn, strValue);
	b.append("record_state", 0);
	b.append("create_date", currentDateTime());
	BSONObj p = b.obj();
	DBconn->insert(strCon, p);
}

string CMongoDBHandler::insert(std::string strDB, std::string strCollection, std::string strJSON)
{
	string strId;

	if (!isValid())
		return strId;

	string strCon = strDB + "." + strCollection;
	BSONObj bson = mongo::fromjson(strJSON);

	try
	{
		BSONObjBuilder tempJson;
		tempJson.genOID();
		tempJson.appendElements(bson);
		tempJson.append("record_state", 0);
		tempJson.append("create_date", currentDateTime());
		bson = tempJson.obj();
		DBconn->insert(strCon, bson);
		BSONElement oi;
		bson.getObjectID(oi);
		OID oid = oi.__oid();
		strId = oid.toString();
	}
	catch (const exception &e)
	{
		_log("[CMongoDBHandler] Insert Data Fail, Error:%s", e.what());
		return strId;
	}
	_log("[CMongoDBHandler] Insert Data to :%s Data:%s", strCon.c_str(), bson.toString().c_str());
	return strId;
}

bool CMongoDBHandler::isValid()
{
	return !DBconn->isFailed();
}

int CMongoDBHandler::query(std::string strDB, std::string strCollection, std::string strField, std::string strCondition,
		std::list<std::string> &listJSON)
{
	if (!isValid())
		return FAIL;

	string strCon = strDB + "." + strCollection;
	_log("[CMongoDBHandler] query , con = %s", strCon.c_str());

	try
	{
		BSONArrayBuilder display_ids;
		//display_ids.append( mongo::OID( "5061f915e4b045bab5e0c957" ) );
		display_ids.append(strCondition);

		BSONObjBuilder in_condition;
		in_condition.append("$in", display_ids.arr());

		BSONObjBuilder message_condition;
		message_condition.append(strField, in_condition.obj());
		//message_condition.append("status", "sending");

		//BSONObjBuilder sortBuilder;
		//sortBuilder.append("_id", 1);

		mongo::Query query(message_condition.obj());
		mongo::Query query2(query);
		//mongo::Query query_with_sort(query.sort(sortBuilder.obj()));

		//	BSONObjBuilder update_field;
		//	update_field.append( "status", "waiting" );
		//BSONObjBuilder set_field;
		//set_field.append( "$set", update_field.obj() );
		//session.update( "mydb.mycoll", query2, set_field.obj(), false, true );
		//con.done();

		BSONObj bsonobj;
		auto_ptr<DBClientCursor> cursor = DBconn->query(strCon, query2);
		while (cursor->more())
		{
			bsonobj = cursor->next();
			listJSON.push_back(bsonobj.jsonString());
		}

	}
	catch (const exception &e)
	{
		_log("[CMongoDBHandler] Query Data Fail, Error:%s", e.what());
		return FALSE;
	}

	return TRUE;
}

int CMongoDBHandler::query(string strDB, string strCollection, string strField, string strFilter, string strCondition,
		list<string> &listJSON)
{
	if (!isValid())
		return FAIL;

	string strCon = strDB + "." + strCollection;
	_log("[CMongoDBHandler] query , con = %s", strCon.c_str());

	mongo::BSONObj query = BSON(
			"create_date" << BSON( strFilter << strCondition ) << "ID" << BSON("$regex" << "1472188091474"));
	//cout << query << endl;
	_log("[CMongoDBHandler] query command: %s", query.toString().c_str());

	auto_ptr<mongo::DBClientCursor> cursor = DBconn->query(strCon, query);
	//cout << "using cursor" << endl;
	//_log("[CMongoDBHandler] query result count: %d", cursor->itcount());
	int nCount = 0;
	while (cursor->more())
	{
		mongo::BSONObj obj = cursor->next();
		++nCount;
		listJSON.push_back(obj.jsonString());
		//cout << "\t" << obj.jsonString() << endl;
	}
	_log("[CMongoDBHandler] query Finish count: %d", nCount);

	return TRUE;
}

