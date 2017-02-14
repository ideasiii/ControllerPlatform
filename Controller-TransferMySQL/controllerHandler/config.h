/*
 * config.h
 *
 *  Created on: 2017年2月2日
 *      Author: Jugo
 */

#pragma once
#include <string>

#define DEFAULT_LAST_DATE			"2015-07-27 00:00:00" // ^_^ Jugo 到職日;
#define TIMER_DU					3

#define APP_ID_POYA_ANDROID		"1472188038304"
#define APP_ID_POYA_IOS			"1472188091474"

struct SETTING_DB
{
	std::string strHost;
	std::string strPort;
	std::string strUser;
	std::string strPassword;
	std::string strDatabase;
};

extern struct SETTING_DB psqlSetting;
extern struct SETTING_DB mysqlSetting;

