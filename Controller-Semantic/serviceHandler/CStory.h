/*
 * CStory.h
 *
 *  Created on: 2017年10月17日
 *      Author: jugo
 */

#pragma once

#include <list>
#include "CSemanticService.h"

class CMysqlHandler;

class CStory: public CSemanticService
{
public:
	CStory();
	virtual ~CStory();
	void init();
	int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch);
	int activity(const char *szInput, JSONObject& jsonResp);
	CString name();

private:
	CMysqlHandler *mysql;
	std::list<CString> listMaterial;
	std::map<std::string, std::string> mapStoryMaterial;
};
