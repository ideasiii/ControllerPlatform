/*
 * Controller.h
 *
 *  Created on: 2019年01月07日
 *      Author: Jugo
 */

#pragma once
#include <vector>
#include <string>
#include "CApplication.h"

class CController: public CApplication
{
public:
	explicit CController();
	virtual ~CController();
	int importDB(const char * szPath);
	void moveFile(const char * szPath);

protected:
	int onCreated(void* nMsqKey);
	int onInitial(void* szConfPath);
	int onFinish(void* nMsqKey);
	void onHandleMessage(Message &message);
	void onTimer(int nId);

private:
	void accessFile();
	void foldScan(const char * szFolderPath, std::vector<std::string> & vFileList);
	void insertDB(std::vector<std::string> & vDataList);

private:
	int mnMsqKey;

};
