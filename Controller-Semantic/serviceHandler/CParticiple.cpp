/*
 * CParticiple.cpp
 *
 *  Created on: 2018年7月26日
 *      Author: Jugo
 */

#include "config.h"
#include "CParticiple.h"
#include "CString.h"
#include "common.h"
#include "CFileHandler.h"
#include "utility.h"
#include "CMysqlHandler.h"

using namespace std;

CParticiple::CParticiple()
{

}

CParticiple::~CParticiple()
{

}

void CParticiple::splitter(const char *szContent, set<string> splitterMark)
{
	CString strContent;

	if(!szContent || splitterMark.empty())
		return;
	strContent = szContent;

	_log("[CParticiple] splitter Content: %s", szContent);
}

void CParticiple::splitter(const char *szPath, const char *szMark)
{
	int nIndex;
	CFileHandler fh;
	set<string> setData;
	CMysqlHandler mysql;
	CString strSQL;
	set<string>::const_iterator iter_set;
	CString strFileName;
	CString strFilePath;
	string strContent;
	set<string> setPhase;

	fh.readPath(szPath, setData);

	if(0 >= setData.size())
	{
		_log("[CParticiple] splitter Path: %s no file", szPath);
		return;
	}

	mysql.connect(EDUBOT_HOST, EDUBOT_DB, EDUBOT_ACCOUNT, EDUBOT_PASSWD, "5");
	if(!mysql.isValid())
	{
		_log("[CParticiple] splitter MySQL invalid");
		return;
	}

	for(iter_set = setData.begin(); setData.end() != iter_set; ++iter_set)
	{
		nIndex = iter_set->rfind(".");
		if((int) string::npos != nIndex)
		{
			if(!iter_set->substr(nIndex + 1).compare("txt"))
			{
				strFileName = trim(iter_set->substr(0, nIndex));
				_log("[CParticiple] splitter Start analysis file: %s", iter_set->c_str());
				strFilePath.format("%s%s", szPath, iter_set->c_str());
				strContent.clear();
				fh.readContent(strFilePath.getBuffer(), strContent);
				setPhase.clear();
				spliteData(const_cast<char*>(strContent.c_str()), szMark, setPhase);

				for(set<string>::iterator it = setPhase.begin(); it != setPhase.end(); ++it)
				{
					_log("[CParticiple] splitter get File: %s Phase: %s", strFileName.getBuffer(), it->c_str());
				}
			}
		}
	}
}

