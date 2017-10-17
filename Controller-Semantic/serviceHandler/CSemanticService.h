/*
 * CService.h
 *
 *  Created on: 2017年10月17日
 *      Author: jugo
 *      CService is a Abstract class
 */

#include <map>
#include <string>
#include "JSONObject.h"
#include "CString.h"

class CSemanticService
{
public:
	virtual ~CSemanticService();

public:
	virtual void init() = 0;
	virtual int evaluate(const char *szWord, std::map<std::string, std::string> &mapMatch) = 0;
	virtual int activity(const char *szInput, JSONObject& jsonResp) = 0;
	virtual CString name() = 0;
};
