/*
 * CRdmLogin.h
 *
 *  Created on: 2016年7月21日
 *      Author: Jugo
 */

#pragma once

#include <string>

using namespace std;

class CRdmLogin
{
public:
	explicit CRdmLogin();
	virtual ~CRdmLogin();
	bool login(const string strAccount, const string strPassword, const string strId, const int nDevice);
	int logout(const string strId);
};
