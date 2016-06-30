/*
 * CAuthentication.h
 *
 *  Created on: 2016年3月30日
 *      Author: Jugo
 */

#pragma once

#include <string>

class CSqliteHandler;

class CAuthentication
{
	public:
		static CAuthentication* getInstance();
		virtual ~CAuthentication();
		bool authorization(const int nServiceType, const std::string strData);

	private:
		CAuthentication();
		CSqliteHandler *sqlite;
};
