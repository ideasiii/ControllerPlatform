/*
 * CAccessLog.h
 *
 *  Created on: 2015年12月7日
 *      Author: Jugo
 *      update: 2016/5/16
 *      		send access log to mongodb controller.
 */

#pragma once
#include <string>

class CSocketClient;
class CCmpHandler;

class CAccessLog
{
	public:
		static CAccessLog* getInstance();
		virtual ~CAccessLog();
		int connectDB(std::string strIP, int nPort);
		int cmpAccessLogRequest(std::string strType, std::string strLog);

	private:
		explicit CAccessLog();
		CSocketClient *mongoClient;
		CCmpHandler *cmpParser;
};
