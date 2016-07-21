/*
 * CRdmLogin.h
 *
 *  Created on: 2016年7月21日
 *      Author: Jugo
 */

#pragma once

#include <string>

class CRdmLogin
{
public:
	explicit CRdmLogin();
	virtual ~CRdmLogin();
	bool login(const std::string strAccount, const std::string strPassword, const std::string strId, const int nDevice);
};
