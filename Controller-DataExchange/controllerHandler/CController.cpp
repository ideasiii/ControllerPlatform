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
#include <mongocxx/instance.hpp>
#include "CFileHandler.h"
#include "CString.h"

#include <cstdint>
#include <iostream>
#include <vector>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

#define PATH_FOLDER		"/data/arx"
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

	mongocxx::instance instance { }; // This should be done only once.

	setTimer(666, 3, 3);

	return nRet;
}

void CController::accessFile()
{
	CString strPath;
	CFileHandler fileHandler;
	vector<string> vFileList;
	vector<string> vCsv;

	foldScan(PATH_FOLDER, vFileList);

	for (vector<string>::iterator vecit = vFileList.begin(); vFileList.end() != vecit; ++vecit)
	{
		strPath.format("%s/%s", PATH_FOLDER, vecit->c_str());
		_log("[CController] accessFile File name: %s", strPath.getBuffer());
		vCsv.clear();
		fileHandler.readAllLine(strPath.getBuffer(), vCsv);
		if (vCsv.size())
		{
			insertDB(vCsv);
		}
	}

	//=======  insert data to monogodb =========//
//	mongocxx::instance inst { };
//	mongocxx::client conn { mongocxx::uri { } };
//
//	bsoncxx::builder::stream::document document { };
//
//	auto collection = conn["testdb"]["testcollection"];

//	document << "hello" << "world";
//	coll.insert_one(document.view());
//	auto cursor = collection.find( { });
//
//	for (auto&& doc : cursor)
//	{
//		std::cout << bsoncxx::to_json(doc) << std::endl;
//	}

}

void CController::insertDB(std::vector<std::string> & vDataList)
{
	mongocxx::uri uri("mongodb://localhost:27017");
	mongocxx::client client(uri);
	mongocxx::database db = client["findata"];
	mongocxx::collection collection = db["csv"];
	vector<bsoncxx::document::value> documents;

	string strColumn = vDataList[0];
	_log("[CController] insertDB get Columns: %s", strColumn.c_str());

//	for (int i = 0; i < 100; i++)
//	{
//		documents.push_back(bsoncxx::builder::stream::document { } << "i" << i << "j" << i + 1 << finalize);
//	}
//	collection.insert_many(documents);
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

void CController::foldScan(const char * szFolderPath, vector<string> & vFileList)
{
	CFileHandler fileHandler;
	vector<string> vFiles;
	int nIndex;

	if (fileHandler.readPath(szFolderPath, vFiles))
	{
		for (vector<string>::iterator it = vFiles.begin(); vFiles.end() != it; ++it)
		{
			nIndex = it->rfind(".");
			if ((int) string::npos != nIndex)
			{
				++nIndex;
				if (!it->substr(nIndex).compare("csv") || !it->substr(nIndex).compare("CSV"))
				{
					vFileList.push_back(*it);
				}
			}
		}
	}
	vFiles.clear();
}
