/*
 * CController.cpp
 *
 *  Created on: 2019年01月07日
 *      Author: Louis Ju
 */

#include <string>
#include <map>
#include "CController.h"
#include "common.h"
#include "event.h"
#include "packet.h"
#include "utility.h"
#include "CConfig.h"
#include <dirent.h>
#include <stdio.h>
#include <set>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include "CFileHandler.h"

using namespace std;

CController::CController() :
		mnMsqKey(-1)
{

}

CController::~CController()
{

}

int CController::onCreated(void* nMsqKey)
{

	mnMsqKey = EVENT_MSQ_KEY_CONTROLLER_DATAEXT;
	return mnMsqKey;
}

int CController::onInitial(void* szConfPath)
{
	int nRet;
	int nPort;
	CConfig *config;
	string strConfPath;
	string strValue;

	nRet = FALSE;
	strConfPath = reinterpret_cast<const char*>(szConfPath);
	_log("[CController] onInitial Config File: %s", strConfPath.c_str());
	if (strConfPath.empty())
		return FALSE;

	setTimer(666, 3, 3);

	return nRet;
}

void CController::accessFile()
{
	vector<string> vFileList;

	foldScan(vFileList);

	//=======  insert data to monogodb =========//
//	mongocxx::instance inst { };
//	mongocxx::client conn { mongocxx::uri { } };
//
//	bsoncxx::builder::stream::document document { };
//
//	auto collection = conn["testdb"]["testcollection"];
//	document << "hello" << "world";
//
//	collection.insert_one(document.view());
//	auto cursor = collection.find( { });
//
//	for (auto&& doc : cursor)
//	{
//		std::cout << bsoncxx::to_json(doc) << std::endl;
//	}
}

void CController::onTimer(int nId)
{
	switch (nId)
	{
	case 666:
		killTimer(666);
		accessFile();
		break;
	}
}

int CController::onFinish(void* nMsqKey)
{
	killTimer(666);
	return TRUE;
}

void CController::onHandleMessage(Message &message)
{

}

void CController::foldScan(vector<string> & vFileList)
{
	CFileHandler fileHandler;
	vector<string> vFiles;
	int nIndex;

	if (fileHandler.readPath("/data/arx", vFiles))
	{
		for (vector<string>::iterator it = vFiles.begin(); vFiles.end() != it; ++it)
		{
			nIndex = it->rfind(".");
			if ((int) string::npos != nIndex)
			{
				++nIndex;
				if (!it->substr(nIndex).compare("csv") || !it->substr(nIndex).compare("CSV"))
				{
				//	_log("[CController] foldScan : %s", it->c_str());
					vFileList.push_back(*it);
				}
			}
		}
	}
	vFiles.clear();
}
