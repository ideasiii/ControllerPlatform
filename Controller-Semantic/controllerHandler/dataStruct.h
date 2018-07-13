/*
 * resource.h
 *
 *  Created on: 2017年8月2日
 *      Author: jugo
 */

#pragma once

#include <string>
#include <set>
#include <map>

using namespace std;

struct CONF
{
	string strPath;
	string strName;
	string strFileType;
	string strHost;
	string strDisplayPath;
	string strDictionary;
	string strWordUnknow;
	string strWordError;
	int nType;
	int nService;
};

struct LOCAL_DATA
{
	string strName;
	string strPath;
	string strType;
	string strHost;
	string strDisplayFile;
};

struct DICTIONARY
{
	string strName;
	string strPath;
	int nType;
};

union UDATA
{
	LOCAL_DATA localData;
	DICTIONARY dictionary;
	UDATA()
	{
	}
	~UDATA()
	{
	}
};

struct RESOURCE
{
	set<string> setMatch;
	UDATA udata;
};

struct VOCABULARY
{
	int nSubject;
	int nVerb;
	VOCABULARY()
	{
		nSubject = 0;
		nVerb = 0;
	}
};

