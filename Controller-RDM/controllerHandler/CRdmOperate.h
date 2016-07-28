/*
 * CRdmOperate.h
 *
 *  Created on: 2016年7月28日
 *      Author: Jugo
 */

#pragma once

#include <string>

using namespace std;

class CRdmOperate
{
public:
	static CRdmOperate * getInstance();
	virtual ~CRdmOperate();
	string getOperate(string strId);

private:
	explicit CRdmOperate();
};
