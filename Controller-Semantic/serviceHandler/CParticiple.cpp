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
#include <vector>

using namespace std;

CParticiple::CParticiple()
{

}

CParticiple::~CParticiple()
{

}

void CParticiple::splitter(const char *szPath, const char *szMark)
{
	int nTmp;
	int nIndex;
	int nIndexPhase;
	CFileHandler fh;
	set<string> setData;
	CMysqlHandler mysql;
	CString strSQL;
	set<string>::const_iterator iter_set;
	CString strFileName;
	CString strFilePath;
	string strContent;
	vector<string> vecPhase;

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
				if(!fh.readContent(strFilePath.getBuffer(), strContent, true))
					continue;

				strSQL.format("DELETE FROM story_affective WHERE story = '%s'", strFileName.getBuffer());
				mysql.sqlExec(strSQL.toString());

				//======= 特殊符號替換 =======//
				strContent = ReplaceAll(strContent, "。”", "”。");
				strContent = ReplaceAll(strContent, "。」", "」。");
				_log("[CParticiple] splitter Content: %s", strContent.c_str());

				//======= 字串分詞 ========//
				int nIndex = 0;
				int nLen = 0;
				int nCount = 0;
				CString strPart;

				while(1)
				{
					if(nCount)
						nIndex = nIndex + nLen + strlen(szMark);
					nLen = strContent.find(szMark, nIndex);
					if((int) string::npos == nLen)
						break;
					nLen = nLen - nIndex;
					printf("index=%d , size=%d\n", nIndex, nLen);
					strPart = strContent.substr(nIndex, nLen);
					printf("part: %s\n", strPart.getBuffer());
					strSQL.format("INSERT INTO story_affective (story,numbering,sentence)VALUES('%s',%d,'%s')", strFileName.getBuffer(),nCount,strPart.getBuffer());
					mysql.sqlExec(strSQL.toString());
					++nCount;
				}
			}
		}
	}

	mysql.close();
}

